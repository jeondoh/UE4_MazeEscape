// Fill out your copyright notice in the Description page of Project Settings.


#include "MazePlayer.h"

#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
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
}

// Called every frame
void AMazePlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 에이밍 줌인/줌아웃
	CameraInterpZoom(DeltaTime);
}

void AMazePlayer::InitalizedData()
{
	CameraDefaultFOV = 0.f; // beginPlay에서 재정의 / 에이밍하지 않을때 카메라 시야
	CameraCurrentFOV = 0.f; // beginPlay에서 재정의 / 카메라 현재 위치
	BaseTurnRate = 45.f; // 좌우회전 (키보드 왼쪽/오른쪽 키)
	BaseLookUpRate = 45.f; // 상하회전 (키보드 위/아래 키)
	CameraZoomedFOV = 35.f; // 에이밍 시 카메라 줌
	ZoomInterpSpeed = 40.f; // 에이밍 확대/축소 Interp속도
	bAiming = false; // 에이밍 줌 여부
}


void AMazePlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AMazePlayer::FireWeapon);
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AMazePlayer::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AMazePlayer::AimingButtonReleased);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AMazePlayer::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMazePlayer::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMazePlayer::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMazePlayer::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
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
		// 파티클 적용
		if(bBeamEnd)
		{
			// 총알 발사 사운드
			if(FireSound)
			{
				UGameplayStatics::PlaySound2D(this, FireSound);
			}
			// BeamEndPoint에 총알 충돌시 효과 적용
			if(ImpactParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, BeamEndPoint);
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
		} // end bBeamEnd
	} // end BarrelSocket
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

bool AMazePlayer::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	FVector2D ViewportSize;
	// 뷰포트 크기를 얻기 위해 GEngine 이용
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize); // FVector2D에 화면 크기로 채움
	}

	// 십자선 위치 가져오기
	FVector2D CrossHairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f); // 화면 중앙 위치 구하기
	CrossHairLocation.Y -= 50.f; // 십자선 위치 = 블루프린트에서 설정한대로 -50

	FVector CrossHairWorldPosition;
	FVector CrossHairWorldDirection;

	// DeprojectScreenToWorld = 화면공간 위치 FVector2D를 world공간 위치 FVector로 변환해줌 CrossHairWorldPosition / CrossHairWorldDirection
	// 2D 화면 좌표를 3D World 공간으로 변환
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrossHairLocation, CrossHairWorldPosition, CrossHairWorldDirection);

	if(bScreenToWorld)
	{
		FHitResult ScreenTraceHit;
		const FVector Start{CrossHairWorldPosition};
		const FVector End{CrossHairWorldPosition + CrossHairWorldDirection * 50000.f};

		OutBeamLocation = End;

		// 1번째 라인 추적 : crosshair의 World 위치 
		// 직선을 이용해 충돌 판정(총알 쏠때)
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);

		// 총알 충돌시
		if(ScreenTraceHit.bBlockingHit)
		{
			OutBeamLocation = ScreenTraceHit.Location;
		}

		// 2번째 라인 추적 : 총구와 목표위치 사이의 물체 
		// 물체가 총구와 BeamEndPoint 사이에 존재할때
		// 조준한곳 사이에 물체가 있을때 EndPoint를 변경해준다.
		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart{MuzzleSocketLocation};
		const FVector WeaponTraceEnd{OutBeamLocation};
		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
		if(WeaponTraceHit.bBlockingHit)
		{
			OutBeamLocation = WeaponTraceHit.Location;
		}
		return true;
	}
	return false;
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
