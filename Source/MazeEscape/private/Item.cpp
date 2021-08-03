// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"

#include "MazePlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Curves/CurveVector.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 변수 초기화
	InitalizedData();
	
	// 컴포넌트 설정
	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ITemMesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(GetRootComponent());

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	// PickupWidget 숨기기
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(false);		
	}
	// 등급별 Star 세팅
	SetSwtichStars();

	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
	// 아이템 상태에 따라 Collision변경
	SetItemProperties(ItemState);
	// CustomDepth(아이템 메테리얼(glow effect)) 초기화
	InitializeCustomDepth();
	// 타이머 경과 > 경과시간은 UpdatePulse에서 사용
	StartPulseTimer();
}

void AItem::OnConstruction(const FTransform& Transform)
{
	if(MaterialInstance)
	{
		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);
		ItemMesh->SetMaterial(MaterialIndex, DynamicMaterialInstance);
	}
	EnableGlowMaterial();
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// EquipInterping 상태일때 커브를 이용한 아이템 획득
	ItemInterp(DeltaTime);
	// 아이템 회전 (bRotate 여부에 따라)
	ItemRotate(DeltaTime);
	// Dynamic 메테리얼의 파라메터값을 Curve로 적용
	UpdatePulse();
}

void AItem::InitalizedData()
{
	ItemName = FString("Default"); // 아이템명
	ItemCount = 0; // 탄약수
	ItemRarity = EItemRarity::EIR_Common; // 아이템 등급
	ItemState = EItemState::EIS_Pickup; // 아이템 상태
	ZCurveTime = 0.7f; // 커브시간
	ItemInterpStartLocation = FVector(0.f); // Interp 시작위치
	CameraTargetLocation = FVector(0.f); // 카메라 앞 interp 타겟 위치
	bInterping = false;
	InterpInitalYawOffset = 0.f;
	bRotate = false;
	RotateSpeed = 40.f;
	ItemType = EItemType::EIT_Etc;
	InterpLocIndex = 0;
	MaterialIndex = 0;
	bCanChangeCustomDepth = true;
	GlowAmount = 150.f;
	FresnelExponent = 3.f;
	FresnelReflectFraction = 4.f;
	PulseCurveTime = 5.f;
	SlotIndex = 0;
	bCharacterInventoryFull = false;
}

void AItem::SetSwtichStars()
{
	// 배열 초기화
	for(int8 i = 0; i <= 5; i++)
	{
		ActiveStars.Add(false);
	}
	
	// 등급별 Star 세팅
	switch(ItemRarity)
	{
		case EItemRarity::EIR_Damaged:
			SetActiveStars(1);
			break;
		case EItemRarity::EIR_Common:
			SetActiveStars(2);
			break;
		case EItemRarity::EIR_UnCommon:
			SetActiveStars(3);
			break;
		case EItemRarity::EIR_Rare:
			SetActiveStars(4);
			break;
		case EItemRarity::EIR_Legendary:
			SetActiveStars(5);
			break;
		default:
			;
	}
}

void AItem::SetActiveStars(int8 Num)
{
	for(int8 i = 1; i <= Num; i++)
	{
		ActiveStars[i] = true;
	}
}

void AItem::ItemRotate(float DeltaTime)
{
	if(bRotate && ItemState == EItemState::EIS_Pickup)
	{
		FRotator NowRotation = GetActorRotation();
		NowRotation.Yaw += DeltaTime * RotateSpeed;
		SetActorRotation(NowRotation);
	}
}

void AItem::SetItemProperties(EItemState State)
{
	switch(State)
	{
	case EItemState::EIS_Pickup:
		// Mesh Properties 설정
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// AreaSphere Properties 설정
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		// CollisionBox Properties 설정
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case EItemState::EIS_Equipped:
		PickupWidget->SetVisibility(false);
		// Mesh Properties 설정
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// AreaSphere Properties 설정
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// CollisionBox Properties 설정
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Falling:
		// Mesh Properties 설정
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetEnableGravity(true);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		// AreaSphere Properties 설정
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// CollisionBox Properties 설정
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_EquipInterping:
		PickupWidget->SetVisibility(false);
		// Mesh Properties 설정
        ItemMesh->SetSimulatePhysics(false);
        ItemMesh->SetEnableGravity(false);
        ItemMesh->SetVisibility(true);
        ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
        ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// AreaSphere Properties 설정
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// CollisionBox Properties 설정
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_PickedUp:
		PickupWidget->SetVisibility(false);
		// Mesh Properties 설정
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(false);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// AreaSphere Properties 설정
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// CollisionBox Properties 설정
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;			
	default:
		;
	}
}

void AItem::UpdatePulse()
{
	float ElapsedTime{};
	FVector CurveValue{};

	switch (ItemState)
	{
		case EItemState::EIS_Pickup:
			if(PulseCurve)
			{
				// 경과시간
				ElapsedTime = GetWorldTimerManager().GetTimerElapsed(PulseTimer);
				// 커브값 (MaterialPulseCurve)
				CurveValue = PulseCurve->GetVectorValue(ElapsedTime);
			}
			break;
		case EItemState::EIS_EquipInterping:
			if(InterpPulseCurve)
			{
				ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInerpTimer);
				CurveValue = InterpPulseCurve->GetVectorValue(ElapsedTime);
			}
			break;
	}
	// 선택한 메테리얼에서 파라메터값 적용
	if(DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowAmount"), CurveValue.X * GlowAmount);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("FresnelExponent"), CurveValue.Y * FresnelExponent);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("FresnelReflectFraction"), CurveValue.Z * FresnelReflectFraction);	
	}
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor)
	{
		AMazePlayer* Player = Cast<AMazePlayer>(OtherActor);
		if(Player)
		{
			Player->IncrementOverlappedItemCount(1);
		}
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(OtherActor)
	{
		AMazePlayer* Player = Cast<AMazePlayer>(OtherActor);
		if(Player)
		{
			Player->IncrementOverlappedItemCount(-1);
			Player->UnHighlightInventorySlot();
		}
	}
}

void AItem::PlayPickupSound(bool bForcePlaySound)
{
	if(InterpPlayer)
	{
		if(bForcePlaySound)
		{
			if(PickupSound)
			{
				UGameplayStatics::PlaySound2D(this, PickupSound);
			}
		}
	}
	else if(InterpPlayer && InterpPlayer->GetShouldPlayPickUpSound())
	{
		InterpPlayer->StartPickUpSoundTimer();
		if(PickupSound)
		{
			UGameplayStatics::PlaySound2D(this, PickupSound);
		}		
	}
}

void AItem::PlayEquipSound(bool bForcePlaySound)
{
	if(InterpPlayer)
	{
		if(bForcePlaySound)
		{
			if(PickupSound)
			{
				UGameplayStatics::PlaySound2D(this, EquipSound);
			}
		}
		else if(InterpPlayer->GetShouldPlayEquipSound())
		{
			InterpPlayer->StartEquipSoundTimer();
			if(PickupSound)
			{
				UGameplayStatics::PlaySound2D(this, EquipSound);
			}			
		}
	}
}

FVector AItem::GetInterpLocation()
{
	if(InterpPlayer == nullptr) return FVector(0.f);

	switch(ItemType)
	{
		case EItemType::EIT_Weapon:
			return InterpPlayer->GetInterpLocation(0).SceneComponent->GetComponentLocation();
		default:
			return InterpPlayer->GetInterpLocation(InterpLocIndex).SceneComponent->GetComponentLocation();
	}
}

void AItem::EnableCustomDepth()
{
	if(bCanChangeCustomDepth)
	{
		ItemMesh->SetRenderCustomDepth(true);		
	}
}

void AItem::DisableCustomDepth()
{
	if(bCanChangeCustomDepth)
	{
		ItemMesh->SetRenderCustomDepth(false);		
	}
}

void AItem::ResetPulseTimer()
{
	StartPulseTimer();
}

void AItem::StartPulseTimer()
{
	if(ItemState == EItemState::EIS_Pickup)
	{
		// 재귀
		GetWorldTimerManager().SetTimer(PulseTimer, this, &AItem::ResetPulseTimer, PulseCurveTime);
	}
}

void AItem::InitializeCustomDepth()
{
	DisableCustomDepth();
}

void AItem::EnableGlowMaterial()
{
	if(DynamicMaterialInstance)
	{
		// 메테리얼의 파라메터를 가져옴
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 0);
	}
}

void AItem::DisableGlowMaterial()
{
	if(DynamicMaterialInstance)
	{
		// 메테리얼의 파라메터를 가져옴
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 1);
	}
}

void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	SetItemProperties(State);
}

void AItem::StartItemCurve(AMazePlayer* SetPlayer, bool bForcePlaySound)
{
	bInterping = true;
	SetItemState(EItemState::EIS_EquipInterping);

	InterpPlayer = SetPlayer;
	ItemInterpStartLocation = GetActorLocation();
	// InterpLocations의 가장 작은 ItemCount를 가진 인덱스
	InterpLocIndex = InterpPlayer->GetInterpLocationIndex();
	// Item Count 1증가
	InterpPlayer->IncrementInterpLocItemCount(InterpLocIndex, 1);
	// 아이템 획득 사운드
	PlayPickupSound(bForcePlaySound);
	// ZCurveTime 동안 ItemInterp 함수가 Tick에 의해 실행됨 >> 아이템 획득 효과를 주기위함
	GetWorldTimerManager().ClearTimer(ItemInerpTimer);
	GetWorldTimerManager().SetTimer(ItemInerpTimer, this, &AItem::FinishInterping, ZCurveTime);

	// 카메라의 회전값과 아이템의 회전값을 차이를 구해 아이템과 카메라의 회전값을 같게 함
	// 아이템을 얻을때 캐릭터가 회전해도 아이템이 보이게 -> Iteminterp 함수에서 사용
	// 캐릭터의 카메라 Y 회전값
	const float CameraRotationYaw{InterpPlayer->GetFollowCamera()->GetComponentRotation().Yaw};
	// 아이템 Y 회전값
	const float ItemRotationYaw{GetActorRotation().Yaw};
	InterpInitalYawOffset = ItemRotationYaw - CameraRotationYaw;
	// Glow 효과 여부 상태
	bCanChangeCustomDepth = false;
}

void AItem::FinishInterping()
{
	bInterping = false;
	if(InterpPlayer)
	{
		InterpPlayer->GetPickupItem(this);
		// Item Count 1감소
		InterpPlayer->IncrementInterpLocItemCount(InterpLocIndex, -1);
		// 인벤토리 효과
		InterpPlayer->UnHighlightInventorySlot();
	}
	SetActorScale3D(FVector(1.f));
	// Glow 효과 disabled 
	bCanChangeCustomDepth = true;
	DisableGlowMaterial();
	DisableCustomDepth();
}

void AItem::ItemInterp(float DeltaTime)
{
	if(!bInterping) return;

	if(InterpPlayer && ItemZCurve)
	{
		// 타이머의 경과 시간을 반환함, 타이머 핸들이 유효하지 않은 경우 -1을 반환
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInerpTimer);
		// 경과시간에 따른 커브값
		const float CurveValue = ItemZCurve->GetFloatValue(ElapsedTime);
		// 아이템 위치
		FVector ItemLocation = ItemInterpStartLocation;
		// 카메라 Interp 위치
		const FVector CameraInterpLocation{GetInterpLocation()};
		// 카메라와 아이템 위치의 Z좌표 거리
		const FVector ItemToCamera{FVector(0.f, 0.f, (CameraInterpLocation - ItemLocation).Z)};
		const float DeltaZ = ItemToCamera.Size();
		const FVector CurrentLocation{ GetActorLocation() };
		
		const float InterpXValue = FMath::FInterpTo(CurrentLocation.X, CameraInterpLocation.X, DeltaTime, 30.0f);
		const float InterpYValue = FMath::FInterpTo(CurrentLocation.Y, CameraInterpLocation.Y, DeltaTime, 30.0f);

		ItemLocation.X = InterpXValue;
		ItemLocation.Y = InterpYValue;
		ItemLocation.Z += CurveValue * DeltaZ;
		SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);

		const FRotator CameraRotation{InterpPlayer->GetFollowCamera()->GetComponentRotation()};
		FRotator ItemRotation{0.f, CameraRotation.Yaw + InterpInitalYawOffset, 0.f};
		SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);

		// 아이템 크기 조정
		if(ItemScaleCurve)
		{
			const float ScaleCurveValue = ItemScaleCurve->GetFloatValue(ElapsedTime);
			// 엑터의 World 스케일 크기
			SetActorScale3D(FVector(ScaleCurveValue, ScaleCurveValue, ScaleCurveValue));
		}
	}
	
}
