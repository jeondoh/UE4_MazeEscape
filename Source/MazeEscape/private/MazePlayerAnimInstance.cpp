// Fill out your copyright notice in the Description page of Project Settings.


#include "MazePlayerAnimInstance.h"

#include "MazePlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UMazePlayerAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if(MazePlayer==nullptr)
	{
		MazePlayer = Cast<AMazePlayer>(TryGetPawnOwner());
	}
	if(MazePlayer)
	{
		// ĳ���� �ӵ�
		FVector Velocity{MazePlayer->GetVelocity()};
		Velocity.Z = 0;
		Speed = Velocity.Size();

		// ĳ���� ���� ����
		bIsInAir = MazePlayer->GetCharacterMovement()->IsFalling();

		// ĳ���� �̵� ����
		bIsAccelerating = false;
		if(MazePlayer->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
			bIsAccelerating = true;
		}

		FRotator AimRotation = MazePlayer->GetBaseAimRotation(); // ���� ��ġ
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MazePlayer->GetVelocity()); // ĳ���� �̵� X��ǥ
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		// ĳ���� �̵��ӵ��� 0�϶� �ִϸ��̼ǿ��� JogStop�� �ٷ� ȣ���ϱ� ������ �Ų����� ����
		if(MazePlayer->GetVelocity().Size() > 0.f)
		{
			LastMovementOffsetYaw = MovementOffsetYaw;			
		}
		/*
		FString OffsetMessage = FString::Printf(TEXT("Movement Offset : %f"), MovementOffsetYaw);
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, OffsetMessage);
		}
		*/
	}
}

void UMazePlayerAnimInstance::NativeInitializeAnimation()
{
	MazePlayer = Cast<AMazePlayer>(TryGetPawnOwner());
}
