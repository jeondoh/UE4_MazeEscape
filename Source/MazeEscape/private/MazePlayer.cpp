// Fill out your copyright notice in the Description page of Project Settings.


#include "MazePlayer.h"

#include "DrawDebugHelpers.h"
#include "Item.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
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
}

void AMazePlayer::InitalizedData()
{
	/* 카메라시야 */
	CameraDefaultFOV = 0.f; // beginPlay에서 재정의 / 에이밍하지 않을때 카메라 시야
	CameraCurrentFOV = 0.f; // beginPlay에서 재정의 / 카메라 현재 위치
	/* 이동 */
	BaseTurnRate = 45.f; // 좌우회전 (키보드 왼쪽/오른쪽 키)
	BaseLookUpRate = 45.f; // 상하회전 (키보드 위/아래 키)
	/* 에이밍 */
	CameraZoomedFOV = 35.f; // 에이밍 시 카메라 줌
	ZoomInterpSpeed = 40.f; // 에이밍 확대/축소 Interp속도
	bAiming = false; // 에이밍 줌 여부
	HipTurnRate = 90.f; // 에이밍 하지 않을때 좌우회전
	HipLookUpRate = 90.f; // 에이밍 하지 않을때 상하회전
	AimingTurnRate = 20.f; // 에이밍시 좌우회전
	AimingLookupRate = 20.f; // 에이밍시 상하회전
	/* 에이밍 > 마우스 감도 */ 
	MouseHipTurnRate = 1.0f; // 조준하지 않을때의 마우스 좌우 감도 
    MouseHipLookUpRate = 1.0f; // 조준하지 않을때의 마우스 상하 감도 
    MouseAimingTurnRate = 0.5f; // 조준할때의 마우스 좌우 감도 
    MouseAimingLookUpRate = 0.5f; // 조준할때의 마우스 상하 감도
	/* 조준선 */
	CrosshairSpreadMultiplier = 0.f; // 조준선 속도
	CrosshairVelocityFactor = 0.f; // 조준선 펼침
	CrosshairInAirFactor = 0.f; // 점프시
	CrosshairAimFactor = 0.f; // 조준시
	CorsshairShootingFactor = 0.f; // 발사시
	/* 사격 후 조준선 변경 */
	ShootTimeDuration = 0.05f;
	bFiringBullet = false;
	/* 자동사격 */
	bShouldFire = true;
	bFireButtonPressed = false;
	AutomaticFireRate = 0.2f;
	/* 아이템 */
	bShouldTraceForItems = false;
}

void AMazePlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AMazePlayer::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AMazePlayer::FireButtonReleased);
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AMazePlayer::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AMazePlayer::AimingButtonReleased);
	PlayerInputComponent->BindAction("Interaction", IE_Pressed, this, &AMazePlayer::InteractionBtnPressed);
	PlayerInputComponent->BindAction("Interaction", IE_Released, this, &AMazePlayer::InteractionBtnRelease);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AMazePlayer::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMazePlayer::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMazePlayer::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMazePlayer::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &AMazePlayer::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMazePlayer::LookUp);
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
	bAiming = true;
}

void AMazePlayer::AimingButtonReleased()
{
	bAiming = false;
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

void AMazePlayer::SetLookRate()
{
	BaseTurnRate = bAiming ? AimingTurnRate : HipTurnRate;
	BaseLookUpRate = bAiming ? AimingLookupRate : HipLookUpRate;
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
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if(BarrelSocket)
	{
		const FTransform SocketTransForm = BarrelSocket->GetSocketTransform(GetMesh());
		if(MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransForm);
		}
		// 총알 발사
		FVector BeamEndPoint;
		bool bBeamEnd = GetBeamEndLocation(SocketTransForm.GetLocation(), BeamEndPoint);

		// 파티클 생성
		// 총알 발사 사운드
		if(FireSound)
		{
			UGameplayStatics::PlaySound2D(this, FireSound);
		}
		// 총알 발사 효과
		if(BeamParticles)
		{
			// 총알 발사 시작과 끝에 BeamParticles(연기) 효과 생성
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransForm.GetLocation());
			if(Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEndPoint); // 이미터 > 타겟 > 타겟이름(Target)
			}
		}
		// 추적 성공시(물체가 있으면)
		if(bBeamEnd)
		{
			// BeamEndPoint에 총알 충돌시 효과 적용
			if(ImpactParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, BeamEndPoint);
			}
		} // end bBeamEnd
	} // end BarrelSocket
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
	// 조준선
	StartCrosshairBulletFire();
}

void AMazePlayer::InteractionBtnPressed()
{
	if(TraceHitItem)
	{
		AWeapon* TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
		SwapWeapon(TraceHitWeapon);
	}
	
}

void AMazePlayer::InteractionBtnRelease()
{
}

bool AMazePlayer::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation, 50000.f);

	if(bCrosshairHit)
	{
		OutBeamLocation = CrosshairHitResult.Location;
	}
	// 2번째 라인 추적 : 총구와 목표위치 사이의 물체 
	// 물체가 총구와 BeamEndPoint 사이에 존재할때
	// 조준한곳 사이에 물체가 있을때 EndPoint를 변경해준다.
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart{MuzzleSocketLocation};
	const FVector StartToEnd{OutBeamLocation - MuzzleSocketLocation};
	const FVector WeaponTraceEnd{MuzzleSocketLocation + StartToEnd * 1.25f};
	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
	if(WeaponTraceHit.bBlockingHit)
	{
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	}
	return false;
}

void AMazePlayer::FireButtonPressed()
{
	bFireButtonPressed = true;
	StartFireTimer();
}

void AMazePlayer::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AMazePlayer::StartFireTimer()
{
	if(bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AMazePlayer::AutoFireReset, AutomaticFireRate);
	}
}

void AMazePlayer::AutoFireReset()
{
	bShouldFire = true;
	if(bFireButtonPressed)
	{
		StartFireTimer();
	}
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
		TraceUnderCrosshairs(ItemTraceResult, OutHitLocation, 400.f);
		if(ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraceResult.Actor);
			if(TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
			}			
			if(TraceHitItemLastFrame)
			{
				// 다른 아이템일 경우
				if(TraceHitItem != TraceHitItemLastFrame)
				{
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				}
			}
			TraceHitItemLastFrame = TraceHitItem;
		}		
	}
	else if(TraceHitItemLastFrame)
	{
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
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

void AMazePlayer::EquipWeapon(AWeapon* WeaponToEquip)
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
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
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

void AMazePlayer::MoveForward(float Value)
{
	if(Controller != nullptr && Value != 0.0f)
	{
		const FRotator Rotation{Controller->GetControlRotation()}; 
		const FRotator YawRotation{FRotator(0, Rotation.Yaw, 0)};

		const FVector Direction{FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X)};
		AddMovementInput(Direction, Value);
	}
}

void AMazePlayer::MoveRight(float Value)
{
	if(Controller != nullptr && Value != 0.0f)
	{
		const FRotator Rotation{Controller->GetControlRotation()}; 
		const FRotator YawRotation{FRotator(0, Rotation.Yaw, 0)};

		const FVector Direction{FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y)};
		AddMovementInput(Direction, Value);
	}
}
