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

		FRotator AimRotation = MazePlayer->GetBaseAimRotation(); // 에임 위치
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MazePlayer->GetVelocity()); // 캐릭터 이동 X좌표
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		// 캐릭터 이동속도가 0일때 애니메이션에서 JogStop을 바로 호출하기 때문에 매끄럽지 못함
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
