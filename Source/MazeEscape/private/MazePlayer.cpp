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
	
	// ī�޶� ����
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f; // ĳ���� �ڿ��� ������ �Ÿ��� �����
	CameraBoom->bUsePawnControlRotation = true; // Player �������� ȸ������
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 50.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// ���콺 ȸ���� ĳ���Ͱ� ������� �ʰ� ����
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// ĳ���� ������ 
	GetCharacterMovement()->bOrientRotationToMovement = false; // �Է��ϴ� �������� ĳ���� ������(�̵� �������� ȸ��)
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // �̵������� ���Ҷ� ȸ���ӵ�
	GetCharacterMovement()->JumpZVelocity = 600.f; // ũ�⸸ŭ ����
	GetCharacterMovement()->AirControl = 0.4f; // ��������
	
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
		// �Ѿ� �߻�
		// ���1
		// FireHitLineTrace(SocketTransForm);
		// ���2
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
	// ����Ʈ ũ�⸦ ��� ���� GEngine �̿�
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize); // FVector2D�� ȭ�� ũ��� ä��
	}
	// ���ڼ� ��ġ ��������
	FVector2D CrossHairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f); // ȭ�� �߾� ��ġ ���ϱ�
	CrossHairLocation.Y -= 50.f; // ���ڼ� ��ġ = �������Ʈ���� �����Ѵ�� -50

	FVector CrossHairWorldPosition;
	FVector CrossHairWorldDirection;

	// DeprojectScreenToWorld = ȭ����� ��ġ FVector2D�� world���� ��ġ FVector�� ��ȯ���� CrossHairWorldPosition / CrossHairWorldDirection
	// 2D ȭ�� ��ǥ�� 3D World �������� ��ȯ
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrossHairLocation, CrossHairWorldPosition, CrossHairWorldDirection);

	if(bScreenToWorld)
	{
		FHitResult ScreenTraceHit;
		const FVector Start{CrossHairWorldPosition};
		const FVector End{CrossHairWorldPosition + CrossHairWorldDirection * 50000.f};
		FVector BeamEndPoint{End};
		// ������ �̿��� �浹 ����(�Ѿ� ��)
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);

		// �Ѿ� �浹��
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
			// �Ѿ� �߻� ���۰� ���� BeamParticles(����) ȿ�� ����
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransForm);
			if(Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEndPoint); // �̹��� > Ÿ�� > Ÿ���̸�(Target)
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
