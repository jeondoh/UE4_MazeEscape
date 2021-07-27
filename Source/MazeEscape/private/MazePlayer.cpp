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
AMazePlayer::AMazePlayer() :
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// 카메라 세팅
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f; // 캐릭터 뒤에서 지정한 거리로 따라옴
	CameraBoom->bUsePawnControlRotation = true; // Player 기준으로 회전설정
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 50.f);

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
}

// Called every frame
void AMazePlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AMazePlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AMazePlayer::FireWeapon);
	
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

void AMazePlayer::FireWeapon()
{
	if(FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if(BarrelSocket)
	{
		const FTransform SocketTransForm = BarrelSocket->GetSocketTransform(GetMesh());
		if(MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransForm);
		}
		// 총알 발사
		// 방법1
		// FireHitLineTrace(SocketTransForm);
		// 방법2
		FireHitToCrossHair(SocketTransForm);
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AMazePlayer::FireHitToCrossHair(FTransform SocketTransForm)
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
		FVector BeamEndPoint{End};
		// 직선을 이용해 충돌 판정(총알 쏠때)
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);

		// 총알 충돌시
		if(ScreenTraceHit.bBlockingHit)
		{
			BeamEndPoint = ScreenTraceHit.Location;
			if(ImpactParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, ScreenTraceHit.Location);
			}
		}
		if(BeamParticles)
		{
			// 총알 발사 시작과 끝에 BeamParticles(연기) 효과 생성
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransForm);
			if(Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEndPoint); // 이미터 > 타겟 > 타겟이름(Target)
			}
		}
	}
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
