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
		// 캐릭터 속도
		FVector Velocity{MazePlayer->GetVelocity()};
		Velocity.Z = 0;
		Speed = Velocity.Size();

		// 캐릭터 점프 여부
		bIsInAir = MazePlayer->GetCharacterMovement()->IsFalling();

		// 캐릭터 이동 여부
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
