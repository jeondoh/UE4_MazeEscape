// Fill out your copyright notice in the Description page of Project Settings.


#include "MazePlayer.h"

#include "Ammo.h"
#include "BulletHitInterface.h"
#include "DrawDebugHelpers.h"
#include "Enemy.h"
#include "EnemyController.h"
#include "Item.h"
#include "ItemStorage.h"
#include "SaveEscapeGame.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Sound/SoundCue.h"

// Sets default values
AMazePlayer::AMazePlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 변수 초기화
	InitalizedData();

	// 카메라 세팅
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f; // 캐릭터 뒤에서 지정한 거리로 따라옴
	CameraBoom->bUsePawnControlRotation = true; // Player 기준으로 회전설정
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	// 마우스 회전시 캐릭터가 따라오지 않게 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	// 캐릭터 움직임 
	GetCharacterMovement()->bOrientRotationToMovement = false; // 입력하는 방향으로 캐릭터 움직임(이동 방향으로 회전)
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // 이동방향을 정할때 회전속도
	GetCharacterMovement()->JumpZVelocity = 600.f; // 크기만큼 점프
	GetCharacterMovement()->AirControl = 0.4f; // 공기저항
	// 캐릭터 손 컴포넌트
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));
	// Interp 위치 조정
	SetInterpComponent();
	InitalizeInterpLocations();
}

// Called when the game starts or when spawned
void AMazePlayer::BeginPlay()
{
	Super::BeginPlay();
	if(FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
	// Default 무기 지정 Socket에 부착
	EquipWeapon(SpawnDefaultWeapon());
	// 인벤토리에 무기 넣기
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);	
	// 아이템 테두리 GLow 효과
	EquippedWeapon->DisableCustomDepth();
	EquippedWeapon->DisableGlowMaterial();
	// Item 클래스에 Player 할당
	EquippedWeapon->SetCharacter(this);
	// TMAP 초기화
	InitializeAmmoMap();
	// 캐릭터 이동속도
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
}

// Called every frame
void AMazePlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 에이밍 줌인/줌아웃
	CameraInterpZoom(DeltaTime);
	// 에이밍 회전변수 설정
	SetLookRate();
	// Crosshair 계산
	CalculateCrosshairSpread(DeltaTime);
	// 캐릭터와 겹치는 아이템 추적
	TraceForItems();
	// 웅크리기/서있기의 캡슐 컴포넌트 크기 변화 Interp
	InterpCapsuleHalfHeight(DeltaTime);
}

void AMazePlayer::InitalizedData()
{
	/* 캐릭터 상태 */
	Health = 1000.f; // 캐릭터 체력
	MaxHealth = 1000.f; // 캐릭터 최대체력
	bDead = false; // 캐릭터 죽음여부
	CurrentCapsuleHalfHeight = 300.f; // 캡슐 컴포넌트 높이 > 현재 크기
	StandingCapsuleHalfHeight = 88.f; // 캡슐 컴포넌트 높이 > 서있을때 크기
	CrouchingCapsuleHalfHeight = 44.f; // 캡슐 컴포넌트 높이 > 웅크릴때 크기
	CombatState = ECombatState::ECS_Unoccupied; // 전투 상태
	BaseMovementSpeed = 650.f; // 평소 걸음속도
	CrouchMovementSpeed = 300.f; // 웅크릴때의 걸음속도
	BaseGroundFriction = 4.f; // 평소 바닥 마찰정도
	CrouchGroundFriction = 100.f; // 웅크릴때의 바닥 마찰정도
	StunChance = 0.1f; // 스턴 확률
	bPlayerStunned = false; // 플레이어 스턴여부
	/* 카메라시야 */
	CameraDefaultFOV = 0.f; // beginPlay에서 재정의 / 에이밍하지 않을때 카메라 시야
	CameraCurrentFOV = 0.f; // beginPlay에서 재정의 / 카메라 현재 위치
	CameraZoomedFOV = 30.f; // 에이밍 시 카메라 줌
	/* 이동 */
	BaseTurnRate = 45.f; // 좌우회전 (키보드 왼쪽/오른쪽 키)
	BaseLookUpRate = 45.f; // 상하회전 (키보드 위/아래 키)
	/* 에이밍 */
	ZoomInterpSpeed = 40.f; // 에이밍 확대/축소 Interp속도
	bAiming = false; // 에이밍 줌 여부
	bAimingBUttonPressed = false; // 에이밍 줌 여부(내부함수에서 사용)
	HipTurnRate = 90.f; // 에이밍 하지 않을때 좌우회전
	HipLookUpRate = 90.f; // 에이밍 하지 않을때 상하회전
	AimingTurnRate = 20.f; // 에이밍시 좌우회전
	AimingLookupRate = 20.f; // 에이밍시 상하회전
	/* 에이밍 > 마우스 감도 */ 
	MouseHipTurnRate = 1.0f; // 조준하지 않을때의 마우스 좌우 감도 
    MouseHipLookUpRate = 1.0f; // 조준하지 않을때의 마우스 상하 감도 
    MouseAimingTurnRate = 0.6f; // 조준할때의 마우스 좌우 감도 
    MouseAimingLookUpRate = 0.6f; // 조준할때의 마우스 상하 감도
	/* 조준선 */
	CrosshairSpreadMultiplier = 0.f; // 조준선 속도
	CrosshairVelocityFactor = 0.f; // 조준선 펼침
	CrosshairInAirFactor = 0.f; // 점프시
	CrosshairAimFactor = 0.f; // 조준시
	CorsshairShootingFactor = 0.f; // 발사시
	/* 사격 후 조준선 변경 */
	ShootTimeDuration = 0.05f; // 사격후 시간
	bFiringBullet = false; // 사격 여부
	/* 자동사격 */
	// bShouldFire = true; // 총 발사 여부
	bFireButtonPressed = false; // 왼쪽 마우스 클릭 여부
	/* 아이템 */
	bShouldTraceForItems = false; // 아이템 추적
	/* 탄약 */	
	Starting9mmAmmo = 85; // 9mm 탄약개수
	StartingARAmmo = 120; // AR 탄약개수
	/* 애니메이션 */
	bCrouching = false; // 웅크리기
	/* Interp 설정 */
	bShouldPlayPickUpSound = true; // 아이템 줍기 사운드 여부
	bShouldPlayEquipSound = true; // 무기 획득 사운드 여부
	PickUpSoundResetTime = 0.2f; // 아이템 줍기 사운드 시간 리셋시간(다른 아이템 픽업 전 대기시간)
	EquipSoundResetTime = 0.2f; // 무기 획득 사운드 시간 리셋시간(다른 아이템 픽업 전 대기시간)
	/* 인벤토리 */
	HighlightedSlot = -1; // 강조 표시(애니메이션)된 슬롯의 인덱스
	/* 레벨1 */
	bKey1 = false;
	bKey2 = false;
	bKey3 = false;
	bKey4 = false;
}

void AMazePlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMazePlayer::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AMazePlayer::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AMazePlayer::FireButtonReleased);
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AMazePlayer::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AMazePlayer::AimingButtonReleased);
	PlayerInputComponent->BindAction("Interaction", IE_Pressed, this, &AMazePlayer::InteractionBtnPressed);
	PlayerInputComponent->BindAction("Interaction", IE_Released, this, &AMazePlayer::InteractionBtnReleased);
	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AMazePlayer::ReloadButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMazePlayer::CrouchButtonPressed);

	PlayerInputComponent->BindAction("1key", IE_Pressed, this, &AMazePlayer::OnekeyPressed);
	PlayerInputComponent->BindAction("2key", IE_Pressed, this, &AMazePlayer::TwokeyPressed);
	PlayerInputComponent->BindAction("3key", IE_Pressed, this, &AMazePlayer::ThreekeyPressed);
	PlayerInputComponent->BindAction("4key", IE_Pressed, this, &AMazePlayer::FourkeyPressed);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AMazePlayer::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMazePlayer::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMazePlayer::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMazePlayer::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &AMazePlayer::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMazePlayer::LookUp);
}

float AMazePlayer::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Health -= DamageAmount;
	if(Health <= 0.f)
	{
		Health = 0.f;
		Die();

		auto EnemyController = Cast<AEnemyController>(EventInstigator);
		if(EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("PlayerDead"), true);			
		}
	}
	return DamageAmount;
}

void AMazePlayer::Die()
{
	bDead = true;
	GetCharacterMovement()->MaxWalkSpeed = 0.f;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
}

void AMazePlayer::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
	APlayerController* APC = UGameplayStatics::GetPlayerController(this, 0);
	if(APC)
	{
		DisableInput(APC);
	}
}

void AMazePlayer::StillAiming()
{
	if(bAimingBUttonPressed)
	{
		Aim();
	}
}

void AMazePlayer::GetPickupItem(AItem* Item)
{
	if(bDead) return;
	// 무기 장착 소리
	if(Item)
	{
		Item->PlayEquipSound();
	}
	AWeapon* Weapon = Cast<AWeapon>(Item);
	if(Weapon)
	{
		// 인벤토리에 자리가 있다면
		if(Inventory.Num() < INVENTORY_CAPACITY)
		{
			Weapon->SetSlotIndex(Inventory.Num()); // 인벤토리 Array에 Index설정
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_PickedUp);
		}
		else
		{
			// 인벤토리가 가득 차있다면 무기스왑
			SwapWeapon(Weapon);
		}
	}
	AAmmo* Ammo = Cast<AAmmo>(Item);
	if(Ammo)
	{
		PickUpAmmo(Ammo);
	}
}

void AMazePlayer::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMazePlayer::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMazePlayer::AimingButtonPressed()
{
	bAimingBUttonPressed = true;
	if(CombatState != ECombatState::ECS_Reloading && CombatState != ECombatState::ECS_Equipping && CombatState != ECombatState::ECS_Stunned)
	{
		Aim();
	}
}

void AMazePlayer::AimingButtonReleased()
{
	bAimingBUttonPressed = false;
	StopAiming();
}

void AMazePlayer::CameraInterpZoom(float DeltaTime)
{
	if(bAiming)
	{	
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AMazePlayer::Turn(float Value)
{
	float TurnScaleFactor{bAiming ? MouseAimingTurnRate : MouseHipTurnRate};
	AddControllerYawInput(Value * TurnScaleFactor);
}

void AMazePlayer::LookUp(float Value)
{
	float TurnScaleFactor{bAiming ? MouseAimingLookUpRate : MouseHipLookUpRate};
	AddControllerPitchInput(Value * TurnScaleFactor);
}

void AMazePlayer::Jump()
{
	if(!bPlayerStunned)
	{
		if(bCrouching)
		{
			bCrouching = false;
			GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
			GetCharacterMovement()->GroundFriction = BaseGroundFriction;
			return;
		}
		ACharacter::Jump();	
	}
}

void AMazePlayer::SetPlayerMovementIfCrouch()
{
	if(bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
		GetCharacterMovement()->GroundFriction = CrouchGroundFriction;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}
}

void AMazePlayer::SetLookRate()
{
	BaseTurnRate = bAiming ? AimingTurnRate : HipTurnRate;
	BaseLookUpRate = bAiming ? AimingLookupRate : HipLookUpRate;
}

void AMazePlayer::Aim()
{
	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AMazePlayer::StopAiming()
{
	bAiming = false;
	if(!bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;		
	}
}

void AMazePlayer::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{0.f, 600.f}; // 캐릭터 속도 범위 지정
	FVector2D VelocityMultiplierRange{0.f, 1.f}; // 캐릭터 속도 범위 백분율
	FVector Velocity{GetVelocity()};
	Velocity.Z = 0.f;

	// 캐릭터가 공중에 있을때 십자선 계산
	// 공중에 있을때 조준선을 벌어지게
	CrosshairInAirFactor = GetCharacterMovement()->IsFalling()
		? FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 10.0f) // 점프할때 느리게 벌어짐
		: FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f); // 착지할때 빠르게 줄어듬

	// 에임 조준시(마우스 우클릭) 십자선 계산
	// 조준시 십자선이 모이게
	CrosshairAimFactor = bAiming
		? FMath::FInterpTo(CrosshairAimFactor, 0.5f, DeltaTime, 30.f) // 조준시 조준선 좁게  
		: FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f); // 조준해제시 조준선 복구

	// 사격 후 조준선 벌어지게
	CorsshairShootingFactor = bFiringBullet
		? FMath::FInterpTo(CorsshairShootingFactor, 0.3f, DeltaTime, 60.f) // 사격시 조준선 벌어지게
		: FMath::FInterpTo(CorsshairShootingFactor, 0.f, DeltaTime, 60.f); // ShootTimeDuration 이후 조준선 복구
	
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size()); // 범위지정 -> 백분율
	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CorsshairShootingFactor; // 0.5 ~ 1.5 사이의 값
}

void AMazePlayer::StartCrosshairBulletFire()
{
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AMazePlayer::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AMazePlayer::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AMazePlayer::GetViewPortCrossHair(FVector2D& ViewportSize, FVector& CrossHairWorldPosition, FVector& CrossHairWorldDirection, bool& bScreenToWorld)
{
	// 뷰포트 크기를 얻기 위해 GEngine 이용
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize); // FVector2D에 화면 크기로 채움
	}

	// 십자선 위치 가져오기
	FVector2D CrossHairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f); // 화면 중앙 위치 구하기
	// CrossHairLocation.Y -= 50.f; // 십자선 위치 = 블루프린트에서 설정한대로 -50

	// DeprojectScreenToWorld = 화면공간 위치 FVector2D를 world공간 위치 FVector로 변환해줌 CrossHairWorldPosition / CrossHairWorldDirection
	// 2D 화면 좌표를 3D World 공간으로 변환
	bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrossHairLocation, CrossHairWorldPosition, CrossHairWorldDirection);
}

void AMazePlayer::FireWeapon()
{
	// 장착된 무기가 없다면
	if(EquippedWeapon == nullptr) return;
	// 캐릭터 상태 확인
	if(CombatState != ECombatState::ECS_Unoccupied) return;
	// 탄약체크
	if(WeaponHasAmmo())
	{
		// 조준선 표기
		StartCrosshairBulletFire();
		// 사격 사운드
		PlayFireSound();
		// 총알 발사 및 파티클
		SendBullet();
		// 사격 몽타주 애니메이션
		PlayGunFireMontage();
		// 탄약수 감소
		EquippedWeapon->DecrementAmmo();
		// 자동사격
		StartFireTimer();
		// 권총일 경우 피스톨 움직이는 효과 적용
		if(EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol)
		{
			EquippedWeapon->StartSlideTimer();
		}
	}
}

void AMazePlayer::PlayFireSound()
{
	// 총알 발사 사운드
	if(EquippedWeapon->GetFireSound())
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->GetFireSound());
	}
}

void AMazePlayer::SendBullet()
{
	// 소켓 확인 후 파티클 추가
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if(BarrelSocket)
	{
		const FTransform SocketTransForm = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());
		if(EquippedWeapon->GetMuzzleFlash())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EquippedWeapon->GetMuzzleFlash(), SocketTransForm);
		}
		// 총알 발사
		FHitResult BeamHitResult;
		bool bBeamEnd = GetBeamEndLocation(SocketTransForm.GetLocation(), BeamHitResult);
		// 추적 성공시(물체가 있으면)
		if(bBeamEnd)
		{
			// 추적 대상중에 BulletHit_Implementation 을 정의한 함수가 있으면 해당 함수 구현부로 이동하여 해당 파티클 실행
			if (BeamHitResult.Actor.IsValid())
			{
				IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(BeamHitResult.Actor.Get());
				if (BulletHitInterface)
				{
					BulletHitInterface->BulletHit_Implementation(BeamHitResult, this, GetController());
				}
				// 추적 대상중 Enemy가 있으면 데미지를 입힘
				TraceEnemyToDamage(BeamHitResult);
			}
			// 구현 함수가 없으면 기본 파티클 효과 적용
			else
			{
				// BeamEndPoint에 총알 충돌시 효과 적용
				if(ImpactParticle)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, BeamHitResult.Location);
				}				
			}
		}
		// 총알 발사 효과
		if(BeamParticles)
		{
			// 총알 발사 시작과 끝에 BeamParticles(연기) 효과 생성
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransForm.GetLocation());
			if(Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location); // 이미터 > 타겟 > 타겟이름(Target)
			}
		}
	} // end BarrelSocket
}

void AMazePlayer::PlayGunFireMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AMazePlayer::InteractionBtnPressed()
{
	if(CombatState != ECombatState::ECS_Unoccupied) return;
	if(TraceHitItem && bShouldTraceForItems)
	{
		if(bAiming)
		{
			StopAiming();
		}
		// 아이템 Z커브
		TraceHitItem->StartItemCurve(this, true);
		TraceHitItem = nullptr;
	}
	
}

void AMazePlayer::InteractionBtnReleased()
{
}

bool AMazePlayer::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult)
{
	FVector OutBeamLocation;
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation, 50000.f);

	if(bCrosshairHit)
	{
		OutBeamLocation = CrosshairHitResult.Location;
	}
	// 2번째 라인 추적 : 총구와 목표위치 사이의 물체 
	// 물체가 총구와 BeamEndPoint 사이에 존재할때
	// 조준한곳 사이에 물체가 있을때 EndPoint를 변경해준다.
	const FVector WeaponTraceStart{MuzzleSocketLocation};
	const FVector StartToEnd{OutBeamLocation - MuzzleSocketLocation};
	const FVector WeaponTraceEnd{MuzzleSocketLocation + StartToEnd * 1.25f};
	GetWorld()->LineTraceSingleByChannel(OutHitResult, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
	if(!OutHitResult.bBlockingHit)
	{
		OutHitResult.Location = OutBeamLocation;
		return false;
	}
	return true;
}

void AMazePlayer::FireButtonPressed()
{
	bFireButtonPressed = true;
	FireWeapon();
}

void AMazePlayer::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AMazePlayer::StartFireTimer()
{
	if(EquippedWeapon == nullptr) return;
	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AMazePlayer::AutoFireReset, EquippedWeapon->GetAutoFireRate());
}

void AMazePlayer::AutoFireReset()
{
	if(CombatState == ECombatState::ECS_Stunned) return;
	
	CombatState = ECombatState::ECS_Unoccupied;
	if(EquippedWeapon == nullptr) return;
	if(WeaponHasAmmo())
	{
		if(bFireButtonPressed && EquippedWeapon->GetAutomatic())
		{
			FireWeapon();			
		}
	}
	else
	{
		// 무기 재장전
		ReloadWeapon();
	}
}

void AMazePlayer::CrouchButtonPressed()
{
	if(!bPlayerStunned)
	{
		if(!GetCharacterMovement()->IsFalling())
		{
			bCrouching = !bCrouching;
		}
		SetPlayerMovementIfCrouch();		
	}
}

void AMazePlayer::FinishEquipping()
{
	if(CombatState == ECombatState::ECS_Stunned) return;
	CombatState = ECombatState::ECS_Unoccupied;
	// 마우스 우측 버튼 클릭하고 있으면 에이밍 지속
	StillAiming();
}

bool AMazePlayer::TraceUnderCrosshairs(FHitResult& OutHitReuslt, FVector& OutHitLocation, float Multiply)
{
	FVector2D ViewPortSize;
	FVector CrossHairWorldPosition;
	FVector CrossHairWorldDirection;
	bool bScreenToWorld;
	GetViewPortCrossHair(ViewPortSize, CrossHairWorldPosition, CrossHairWorldDirection, bScreenToWorld);

	if(bScreenToWorld)
	{
		const FVector Start{CrossHairWorldPosition};
		const FVector End{Start + CrossHairWorldDirection * Multiply};
		OutHitLocation = End;

		GetWorld()->LineTraceSingleByChannel(OutHitReuslt, Start, End, ECollisionChannel::ECC_Visibility);
		if(OutHitReuslt.bBlockingHit)
		{
			OutHitLocation = OutHitReuslt.Location;
			return true;
		}
	}
	return false;
}

void AMazePlayer::TraceForItems()
{
	if(bShouldTraceForItems)
	{
		// 무기에 에임을 대면 무기 위젯 활성화
		FHitResult ItemTraceResult;
		FVector OutHitLocation;
		TraceUnderCrosshairs(ItemTraceResult, OutHitLocation, 1000.f);
		if(ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraceResult.Actor);
			// 인벤토리 슬롯 하이라이트(애니메이션) 여부
			SetHighlightInventorySlot();
			
			if(TraceHitItem)
			{
				if(TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
				{
					TraceHitItem = nullptr;
				}
				if(TraceHitItem->GetPickupWidget())
				{
					TraceHitItem->GetPickupWidget()->SetVisibility(true);
					TraceHitItem->EnableCustomDepth();
					// 인벤토리 공간
					bool bInventoryFull = Inventory.Num() >= INVENTORY_CAPACITY ? true : false; 
					TraceHitItem->SetCharacterInventoryFull(bInventoryFull);
				}
			}
			if(TraceHitItemLastFrame)
			{
				// 다른 아이템일 경우
				if(TraceHitItem != TraceHitItemLastFrame)
				{
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
					TraceHitItemLastFrame->DisableCustomDepth();
				}
			}
			TraceHitItemLastFrame = TraceHitItem;
		}		
	}
	else if(TraceHitItemLastFrame)
	{
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		TraceHitItemLastFrame->DisableCustomDepth();
	}
}

AWeapon* AMazePlayer::SpawnDefaultWeapon()
{
	if(DefaultWeaponClass)
	{
		// 무기 스폰
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}
	return nullptr;
}

void AMazePlayer::EquipWeapon(AWeapon* WeaponToEquip, bool bSwaping)
{
	if(WeaponToEquip)
	{
		// 스켈레톤 -> 소켓 가져옴
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if(HandSocket)
		{
			// 해당 소켓에 무기 부착
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}
		if(EquippedWeapon == nullptr)
		{
			// -1 = 장착한 무기 없음. 아이콘 애니메이션 필요 X
			EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
		}
		else if(!bSwaping)
		{
			EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
		}
		
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AMazePlayer::DropWeapon()
{
	if(EquippedWeapon)
	{
		// 착용하고 있는 무기가 있다면 무기 매쉬 분리
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AMazePlayer::SwapWeapon(AWeapon* WeaponToSwap)
{
	if(Inventory.Num() -1 >= EquippedWeapon->GetSlotIndex())
	{
		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
		WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
	}
	DropWeapon();
	EquipWeapon(WeaponToSwap, true);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

void AMazePlayer::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AMazePlayer::WeaponHasAmmo()
{
	if(EquippedWeapon == nullptr) return false;
	if(EquippedWeapon->GetAmmo() > 0) return true;
	// 총알없을때 사운드
	auto AmmoType = EquippedWeapon->GetAmmoType(); 
	int32 CarriedAmmo = AmmoMap[AmmoType]; // 전체 총알 개수
	bool hasAmmo = CarriedAmmo == 0;
	if(hasAmmo && EmptyBulletSound)
	{
		UGameplayStatics::PlaySound2D(this, EmptyBulletSound);
	}
	return false;
}

void AMazePlayer::ReloadButtonPressed()
{
	if(CombatState!=ECombatState::ECS_Unoccupied) return;
	if(EquippedWeapon == nullptr) return;
	ReloadWeapon();
}

void AMazePlayer::ReloadWeapon()
{
	// 총기에 맞는 탄약 확인
	if(CarryingAmo() && !EquippedWeapon->ClipIsFull())
	{
		if(bAiming)
		{
			StopAiming();
		}
		CombatState = ECombatState::ECS_Reloading;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if(AnimInstance && ReloadMontage)
		{
			// 몽타주 실행
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
		}
	}
	
}

void AMazePlayer::FinishedReload()
{
	if(CombatState == ECombatState::ECS_Stunned) return;
	
	// AmmoMap 업데이트
	CombatState = ECombatState::ECS_Unoccupied;
	// 마우스 우측 버튼 클릭하고 있으면 에이밍 지속
	StillAiming();
	
	if(EquippedWeapon == nullptr) return;

	const auto AmmoType{EquippedWeapon->GetAmmoType()};
	
	if(AmmoMap.Contains(AmmoType))
	{
		// 캐릭터가 가지고 있는 탄약 수
		int32 CarriedAmmo = AmmoMap[AmmoType];
		// 탄창용량(빈탄창) - 탄약수 (탄창에 남은 공간)
		const int32 MagEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();
		// 탄창에 남은 공간 > 가지고 있는 탄약 수
		if(MagEmptySpace > CarriedAmmo)
		{
			// 가지고 있는 탄약 Reload
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
		}
		else
		{
			// 탄챵 채우기
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
		}
		AmmoMap.Add(AmmoType, CarriedAmmo);
	}
}

void AMazePlayer::GrabClip()
{
	if(EquippedWeapon == nullptr) return;
	if(HandSceneComponent == nullptr) return;
	
	// 탄창 Transform(위치, 회전, 스케일)
	int32 ClipBoneIndex{EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName())};
	ClipTransfrom = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);
	// 손에 탄창 부착
	// KeepRelative = 손에 탄창 오프셋 유지
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName("Hand_l"));
	HandSceneComponent->SetWorldTransform(ClipTransfrom);

	EquippedWeapon->SetClipBoneName(true);
}

void AMazePlayer::ReleaseClip()
{
	EquippedWeapon->SetClipBoneName(false);	
}

void AMazePlayer::PickUpAmmo(AAmmo* Ammo)
{
	if(AmmoMap.Find(Ammo->GetAmmoType()))
	{
		// 탄약수
		int32 AmmoCount = AmmoMap[Ammo->GetAmmoType()];
		AmmoCount += Ammo->GetItemCount();
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}
	// 총기와 탄약이 일치할때
	if(EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		// 탄약이 없을때 자동 리로드
		if(EquippedWeapon->GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}
	Ammo->Destroy();
}

bool AMazePlayer::CarryingAmo()
{
	if(EquippedWeapon == nullptr) return false;

	EAmmoType AmmoType = EquippedWeapon->GetAmmoType();
	if(AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}
	
	return false;
}

int32 AMazePlayer::GetInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;
	int32 InterpSize = InterpLocations.Num();
	
	for(int32 i = 1; i < InterpSize; i++)
	{
		if(InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount; 
		}
	}
	return LowestIndex;
}

void AMazePlayer::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
	if(Amount < -1 || Amount > 1) return;
	if(InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
}

void AMazePlayer::StartPickUpSoundTimer()
{
	bShouldPlayPickUpSound = false;
	GetWorldTimerManager().SetTimer(PickUpSoundTimer, this, &AMazePlayer::ResetPickupSoundTimer, PickUpSoundResetTime);
}

void AMazePlayer::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;
	GetWorldTimerManager().SetTimer(EquipSoundTimer, this, &AMazePlayer::ResetEquipSoundTimer, EquipSoundResetTime);
}

void AMazePlayer::InitalizeInterpLocations()
{
	FInterpLocation WeaponLocation{WeaponInterpComp, 0};
	InterpLocations.Add(WeaponLocation);
	
	FInterpLocation InterpLoc1{InterpComp1, 0};
	InterpLocations.Add(InterpLoc1);

	FInterpLocation InterpLoc2{InterpComp2, 0};
	InterpLocations.Add(InterpLoc2);

	FInterpLocation InterpLoc3{InterpComp3, 0};
	InterpLocations.Add(InterpLoc3);

	FInterpLocation InterpLoc4{InterpComp4, 0};
	InterpLocations.Add(InterpLoc4);

	FInterpLocation InterpLoc5{InterpComp5, 0};
	InterpLocations.Add(InterpLoc5);

	FInterpLocation InterpLoc6{InterpComp6, 0};
	InterpLocations.Add(InterpLoc6);
}

void AMazePlayer::SetInterpComponent()
{
	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon InterpComp"));
	WeaponInterpComp->SetupAttachment(GetFollowCamera());
	
	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp1"));
	InterpComp1->SetupAttachment(GetFollowCamera());

	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp2"));
	InterpComp2->SetupAttachment(GetFollowCamera());

	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp3"));
	InterpComp3->SetupAttachment(GetFollowCamera());

	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp4"));
	InterpComp4->SetupAttachment(GetFollowCamera());

	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp5"));
	InterpComp5->SetupAttachment(GetFollowCamera());

	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp6"));
	InterpComp6->SetupAttachment(GetFollowCamera());
}

void AMazePlayer::ResetPickupSoundTimer()
{
	bShouldPlayPickUpSound = true;
}

void AMazePlayer::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}

void AMazePlayer::IncrementOverlappedItemCount(int8 Amount)
{
	if(OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
		return;
	}
	OverlappedItemCount += Amount;
	bShouldTraceForItems = true;
}

FInterpLocation AMazePlayer::GetInterpLocation(int32 Index)
{
	if(Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];
	}
	return FInterpLocation();
}

void AMazePlayer::Stun()
{
	if(Health <= 0.f) return;
	
	CombatState = ECombatState::ECS_Stunned;

	if(bAimingBUttonPressed)
	{
		StopAiming();
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
	}
}

void AMazePlayer::EndStun()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if(bAimingBUttonPressed)
	{
		Aim();
	}
}

void AMazePlayer::InterpCapsuleHalfHeight(float DeltaTime)
{
	float TargetCapsuleHalfHeight{bCrouching ? CrouchingCapsuleHalfHeight : StandingCapsuleHalfHeight};
	const float InterpHalfHeight{FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
		TargetCapsuleHalfHeight, DeltaTime, 20.f)};

	const float DeltaCapsuleHalfHeight{InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()};
	const FVector MeshOffset{0.f, 0.f, -DeltaCapsuleHalfHeight};
	GetMesh()->AddLocalOffset(MeshOffset);
	
	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

void AMazePlayer::MoveForward(float Value)
{
	if(Controller != nullptr && !bPlayerStunned && Value != 0.0f)
	{
		const FRotator Rotation{Controller->GetControlRotation()}; 
		const FRotator YawRotation{FRotator(0, Rotation.Yaw, 0)};

		const FVector Direction{FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X)};
		AddMovementInput(Direction, Value);
	}
}

void AMazePlayer::MoveRight(float Value)
{
	if(Controller != nullptr && !bPlayerStunned && Value != 0.0f)
	{
		const FRotator Rotation{Controller->GetControlRotation()}; 
		const FRotator YawRotation{FRotator(0, Rotation.Yaw, 0)};

		const FVector Direction{FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y)};
		AddMovementInput(Direction, Value);
	}
}

int32 AMazePlayer::GetEmptyInventorySlot()
{
	int32 InventorySize = Inventory.Num();
	for(int32 i = 0; i < InventorySize; i++)
	{
		if(Inventory[i] == nullptr)
		{
			return i; // 빈자리 인덱스 반환
		}
	}
	if(InventorySize < INVENTORY_CAPACITY)
	{
		return InventorySize;
	}
	return -1; // 빈자리가 없음
}

void AMazePlayer::SetHighlightInventorySlot()
{
	auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
	if (TraceHitWeapon)
	{
		if (HighlightedSlot == -1)
		{
			HighlightInventorySlot();
		}
	}
	else
	{
		if (HighlightedSlot != -1)
		{
			// 하이라이트 애니메이션 X
			UnHighlightInventorySlot();
		}
	}
}

void AMazePlayer::HighlightInventorySlot()
{
	const int32 EmptySlot{GetEmptyInventorySlot()}; // 빈 인벤토리 인덱스
	HighlightIconDelegate.Broadcast(EmptySlot, true);
	HighlightedSlot = EmptySlot;
}

void AMazePlayer::UnHighlightInventorySlot()
{
	HighlightIconDelegate.Broadcast(HighlightedSlot, false);
	HighlightedSlot = -1;
}

void AMazePlayer::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
	if(CurrentItemIndex != NewItemIndex &&
		NewItemIndex < Inventory.Num() &&
		(CombatState == ECombatState::ECS_Unoccupied || CombatState == ECombatState::ECS_Equipping))
	{
		if(bAiming)
		{
			StopAiming();
		}
		auto OldEquippedWeapon = EquippedWeapon;
		auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);
		EquipWeapon(NewWeapon);

		OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
		NewWeapon->SetItemState(EItemState::EIS_Equipped);

		CombatState = ECombatState::ECS_Equipping;
	
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if(AnimInstance && EquippedMontage)
		{
			AnimInstance->Montage_Play(EquippedMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("Equip"));
		}
		NewWeapon->PlayEquipSound(true);
	}
}

void AMazePlayer::OnekeyPressed()
{
	KeyPressedToEquipped(0);
}

void AMazePlayer::TwokeyPressed()
{
	KeyPressedToEquipped(1);
}

void AMazePlayer::ThreekeyPressed()
{
	KeyPressedToEquipped(2);
}

void AMazePlayer::FourkeyPressed()
{
	KeyPressedToEquipped(3);
}

void AMazePlayer::KeyPressedToEquipped(int32 SlotIndex)
{
	if(EquippedWeapon->GetSlotIndex() == SlotIndex) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), SlotIndex);
}

EPhysicalSurface AMazePlayer::GetSurfaceType()
{
	FHitResult HitResult;
	const FVector Start{GetActorLocation()};
	const FVector End{Start + FVector(0.f, 0.f, -400.f)};
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	// 발이 닿이는 actor 추적
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);
	// LineTrace로 추적되는 메테리얼의 표면을(Physical Surface) 알수 있음
	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
}

void AMazePlayer::TraceEnemyToDamage(FHitResult BeamHitResult)
{
	AEnemy* HitEnemy = Cast<AEnemy>(BeamHitResult.Actor.Get());
	if(HitEnemy)
	{
		// 몬스터 죽으면 데미지 X
		if(HitEnemy->GetIsDead()) return;
		// 맞은 부위에 따라 다른 데미지 설정 (데미지는 데이터테이블에서 설정함)
		FString BoneName = BeamHitResult.BoneName.ToString(); // 총알에 맞은 부위의 스켈레톤 명칭
		FString CustomBoneName = HitEnemy->GetHeadBone(); // 사용자가 Enemy 블루프린트에서 설정한 뼈 이름(머리)
		bool bHeadShot = BoneName == CustomBoneName ? true : false;
		float Damage = BoneName == CustomBoneName
			? RandomizationDamage(EquippedWeapon->GetHeadShotDamage(), bHeadShot)
			: RandomizationDamage(EquippedWeapon->GetDamage(), bHeadShot);
		
		UGameplayStatics::ApplyDamage(BeamHitResult.Actor.Get(), Damage, GetController(),
			this, UDamageType::StaticClass());
		// 데미지 UI 보여주기
		HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location, bHeadShot);
	}
}

float AMazePlayer::RandomizationDamage(float Damage, bool isHeadShot)
{
	float Min = isHeadShot ? Damage - 5.f : Damage - 8.f;
	float Max = isHeadShot ? Damage + 5.f : Damage + 3.f;
	return FMath::RandRange(Min, Max);
}

void AMazePlayer::SwitchLevel(FName LevelName)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FString CurrentLevel = World->GetMapName();
		CurrentLevel.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

		FName CurrentLevelName(*CurrentLevel);
		if (CurrentLevelName != LevelName)
		{
			FString Level = LevelName.ToString();
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}

void AMazePlayer::SaveGame()
{
	USaveEscapeGame* GameInstance = Cast<USaveEscapeGame>(UGameplayStatics::CreateSaveGameObject(USaveEscapeGame::StaticClass()));
	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	
	GameInstance->MazePlayerData.Health = Health;
	GameInstance->MazePlayerData.MaxHealth = MaxHealth;
	GameInstance->MazePlayerData.LevelName = MapName;
	if(EquippedWeapon)
	{
		GameInstance->MazePlayerData.WeaponName = EquippedWeapon->GetWeaponName();
	}
	UGameplayStatics::SaveGameToSlot(GameInstance, GameInstance->PlayerName, GameInstance->UserIndex);
}

void AMazePlayer::LoadGame()
{
	USaveEscapeGame* GameInstance = Cast<USaveEscapeGame>(UGameplayStatics::CreateSaveGameObject(USaveEscapeGame::StaticClass()));
	USaveEscapeGame* LoadGameInstance = Cast<USaveEscapeGame>(UGameplayStatics::LoadGameFromSlot(GameInstance->PlayerName, GameInstance->UserIndex));

	if(LoadGameInstance)
	{
		if (LoadGameInstance->MazePlayerData.LevelName != "")
		{
			FName Map(*LoadGameInstance->MazePlayerData.LevelName);
			// SwitchLevel(Map);
		}
		Health = LoadGameInstance->MazePlayerData.Health;
		MaxHealth = LoadGameInstance->MazePlayerData.MaxHealth;

		if (WeaponStorge)
		{
			AItemStorage* Container = GetWorld()->SpawnActor<AItemStorage>(WeaponStorge);
			if (Container)
			{
				FString WeaponName = GameInstance->MazePlayerData.WeaponName;
				if (Container->WeaponMap.Num() > 0)
				{
					if (Container->WeaponMap.Contains(WeaponName))
					{
						AWeapon* Weapon = GetWorld()->SpawnActor<AWeapon>(Container->WeaponMap[WeaponName]);
						if (Weapon)
						{
							Inventory.Add(EquippedWeapon);
						}
					}
				}
			}
		}
	}
}

void AMazePlayer::DeleteGame()
{
	USaveEscapeGame* GameInstance = Cast<USaveEscapeGame>(UGameplayStatics::CreateSaveGameObject(USaveEscapeGame::StaticClass()));
	UGameplayStatics::DeleteGameInSlot(GameInstance->PlayerName, GameInstance->UserIndex);	
	
}

void AMazePlayer::OpenLevel(FName LevelName)
{
	UWorld* World = GetWorld();
	UGameplayStatics::OpenLevel(World, LevelName);
}

bool AMazePlayer::KeyDoorOpen(FString KeyName, bool bkey)
{
	if(KeyName == "DoorKey_Gold" && bkey)
	{
		return true;
	}
	if(KeyName == "DoorKey_Blue" && bkey)
	{
		return true;
	}
	if(KeyName == "DoorKey_Green" && bkey)
	{
		return true;
	}
	if(KeyName == "DoorKey_Red" && bkey)
	{
		return true;
	}
	return false;
}
