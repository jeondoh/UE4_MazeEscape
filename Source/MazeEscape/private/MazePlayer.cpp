// Fill out your copyright notice in the Description page of Project Settings.


#include "MazePlayer.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

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

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
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
