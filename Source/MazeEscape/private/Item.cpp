// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"

#include "MazePlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

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
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// EquipInterping 상태일때 커브를 이용한 아이템 획득
	ItemInterp(DeltaTime);

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
	ItemInterpX = 0.f;
	ItemInterpY = 0.f;
	InterpInitalYawOffset = 0.f;
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
		
	default:
		;
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
		}
	}
}

void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	SetItemProperties(State);
}

void AItem::StartItemCurve(AMazePlayer* SetPlayer)
{
	InterpPlayer = SetPlayer;
	ItemInterpStartLocation = GetActorLocation();

	SetItemState(EItemState::EIS_EquipInterping);
	bInterping = true;
	// ZCurveTime 동안 ItemInterp 함수가 Tick에 의해 실행됨 >> 아이템 획득 효과를 주기위함 
	GetWorldTimerManager().SetTimer(ItemInerpTimer, this, &AItem::FinishInterping, ZCurveTime);

	// 카메라의 회전값과 아이템의 회전값을 차이를 구해 아이템과 카메라의 회전값을 같게 함
	// 아이템을 얻을때 캐릭터가 회전해도 아이템이 보이게 -> Iteminterp 함수에서 사용
	// 캐릭터의 카메라 Y 회전값
	const float CameraRotationYaw{InterpPlayer->GetFollowCamera()->GetComponentRotation().Yaw};
	// 아이템 Y 회전값
	const float ItemRotationYaw{GetActorRotation().Yaw};
	InterpInitalYawOffset = ItemRotationYaw - CameraRotationYaw;
}

void AItem::FinishInterping()
{
	bInterping = false;
	if(InterpPlayer)
	{
		InterpPlayer->GetPickupItem(this);
	}
	SetActorScale3D(FVector(1.f));
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
		const FVector CameraInterpLocation{InterpPlayer->GetCameraInterpLocation()};
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
