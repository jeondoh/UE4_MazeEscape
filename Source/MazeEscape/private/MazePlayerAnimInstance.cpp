// Fill out your copyright notice in the Description page of Project Settings.


#include "MazePlayerAnimInstance.h"

#include "MazePlayer.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	}
}

void UMazePlayerAnimInstance::NativeInitializeAnimation()
{
	MazePlayer = Cast<AMazePlayer>(TryGetPawnOwner());
}
