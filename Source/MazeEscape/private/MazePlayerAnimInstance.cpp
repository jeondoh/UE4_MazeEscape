// Fill out your copyright notice in the Description page of Project Settings.


#include "MazePlayerAnimInstance.h"

#include "MazePlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UMazePlayerAnimInstance::UMazePlayerAnimInstance()
{
	Speed = 0.f;
	bIsInAir = false;
	bIsAccelerating = false;
	MovementOffsetYaw = 0.f;
	LastMovementOffsetYaw = 0.f;
	bAiming = false;
	TIPCharacterYaw = 0.f;
	TIPCharacterYawLastFrame = 0.f;
	RootYawOffset = 0.f;
	Pitch = 0.f;
	bReloading = false;
	YawDelta = 0.f;
	OffsetState = EOffsetState::EOS_Hip;
	CharacterRotation = FRotator(0.f);
	CharacterRotationLastFrame = FRotator(0.f);
}

void UMazePlayerAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if(MazePlayer==nullptr)
	{
		MazePlayer = Cast<AMazePlayer>(TryGetPawnOwner());
	}
	if(MazePlayer)
	{
		// 재장전여부
		bReloading = MazePlayer->GetCombatState() == ECombatState::ECS_Reloading;
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

		bAiming = MazePlayer->GetAiming(); // 에이밍 여부
		SetEOffsetState();
	}
	TurnInPlace();
	Lean(DeltaTime);
}

void UMazePlayerAnimInstance::NativeInitializeAnimation()
{
	MazePlayer = Cast<AMazePlayer>(TryGetPawnOwner());
}

void UMazePlayerAnimInstance::TurnInPlace()
{
	if(MazePlayer==nullptr) return;

	// 조준위치의 Pitch값
	Pitch = MazePlayer->GetBaseAimRotation().Pitch;
	
	if(Speed > 0 || bIsInAir)
	{
		RootYawOffset = 0.f;
		TIPCharacterYaw = MazePlayer->GetActorRotation().Yaw;
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		RotationCurveLastFrame = 0.f;
		RotationCurve = 0.f;
	}
	else
	{
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		TIPCharacterYaw = MazePlayer->GetActorRotation().Yaw;
		const float TIPYawDelta{ TIPCharacterYaw - TIPCharacterYawLastFrame };

		// Root Yaw 오프셋, [-180, 180] 고정
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);
		
		// 애니메이션 커브에서 추가한 Turning값 가져옴
		// Turning = 애니메이션 실행시 1 Return
		const float Turning{ GetCurveValue(TEXT("Turning")) };
		if (Turning > 0)
		{
			RotationCurveLastFrame = RotationCurve;
			// 애니메이션 커브에서 추가한 Rotation값 가져옴
			RotationCurve = GetCurveValue(TEXT("Rotation"));
			const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };
			// RootYawOffset이 0보다 크면 캐릭터 좌회전
			// RootYawOffset이 0보다 작으면 캐릭터 우회전
			RootYawOffset > 0.f ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;
			
			const float ABSRootYawOffset{ FMath::Abs(RootYawOffset) };
			if (ABSRootYawOffset > 90.f)
			{
				const float YawExcess{ ABSRootYawOffset - 90.f };
				RootYawOffset > 0.f ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
		if (GEngine) GEngine->AddOnScreenDebugMessage(1, -1, FColor::Cyan, FString::Printf(TEXT("RootYawOffset: %f"), RootYawOffset));
	}
}

void UMazePlayerAnimInstance::Lean(float DeltaTime)
{
	if(MazePlayer == nullptr) return;
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = MazePlayer->GetActorRotation();

	const FRotator Delta{UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame)};
	
	const float Target{Delta.Yaw / DeltaTime};
	const float Interp{FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f)};
	// 범위지정 -90 ~ 90
	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);

	if(GEngine) GEngine->AddOnScreenDebugMessage(2, -1, FColor::Green, FString::Printf(TEXT("DeltaYaw :: %f"), Delta.Yaw));
}

void UMazePlayerAnimInstance::SetEOffsetState()
{
	if(bReloading)
	{
		OffsetState = EOffsetState::EOS_Reloading;
	}
	else if(bIsInAir)
	{
		OffsetState = EOffsetState::EOS_InAir;
	}
	else if(MazePlayer->GetAiming())
	{
		OffsetState = EOffsetState::EOS_Aiming;
	}
	else
	{
		OffsetState = EOffsetState::EOS_Hip;
	}
}
