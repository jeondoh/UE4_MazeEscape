// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MazePlayerAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8
{
	EOS_Aiming UMETA(DisplayName = "Aiming"),
	EOS_Hip UMETA(DisplayName = "Hip"),
	EOS_Reloading UMETA(DisplayName = "Reloading"),
	EOS_InAir UMETA(DisplayName = "InAir"),

	EOS_MAX UMETA(DisplayName = "DefaultMax"),
};

/**
* 
*/
UCLASS()
class MAZEESCAPE_API UMazePlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UMazePlayerAnimInstance();
	
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);
	
	virtual void NativeInitializeAnimation() override;

protected:
	void TurnInPlace();

	// 달리는 동안 기울이기에 대한 계산 처리
	void Lean(float DeltaTime);

	// 장전속도 & 총기반동 정도 적용
	void SetRecoilWeight();

	void SetEOffsetState();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	class AMazePlayer* MazePlayer;

	// 캐릭터 이동속도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	float Speed;

	// 캐릭터가 점프중인지 확인
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bIsInAir;

	// 캐릭터가 움직이고 있는지 확인
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bIsAccelerating;

	// 에이밍 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bAiming;

	// 오프셋 YAW > 에임위치 & 캐릭터이동좌표의 차이
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(AllowPrivateAccess=true))
	float MovementOffsetYaw;

	// 캐릭터 이동속도가 0이 아닌 마지막 오프셋 값 저장
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(AllowPrivateAccess=true))
	float LastMovementOffsetYaw;

	// 현재 프레임의 캐릭터의 Yaw 값 / 정지 상태 & 공중상태이지 않을때 업데이트
	float TIPCharacterYaw;
 
	//이전 프레임의 캐릭터의 Yaw 값 / 정지 상태 & 공중상태이지 않을때 업데이트 
	float TIPCharacterYawLastFrame;

	// 현재 프레임의 캐릭터 Rotation값 / 정지 상태에서만 업데이트
	FRotator CharacterRotation;

	// 이전 프레임의 캐릭터 Rotation값 / 정지 상태에서만 업데이트
	FRotator CharacterRotationLastFrame;

	void CharacterStateUpdate();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Lean", meta=(AllowPrivateAccess=true))
	float YawDelta;

	// 스켈레톤 > Root뼈 오프셋
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sight", meta=(AllowPrivateAccess=true))
	float RootYawOffset;

	// 현재 프레임의 Curve값
	float RotationCurve;

	// 이전 프레임의 Curve값
	float RotationCurveLastFrame;

	// 조준위치의 Pitch값
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sight", meta=(AllowPrivateAccess=true))
	float Pitch;	

	// 재장전여부(재장전 동안 오프셋 방지)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sight", meta=(AllowPrivateAccess=true))	
	bool bReloading;

	// 오프셋 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", meta=(AllowPrivateAccess=true))
	EOffsetState OffsetState;
	
	// 장비스왑 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", meta=(AllowPrivateAccess=true))
	bool bEquipping;

	// 웅크리기 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Crouching", meta=(AllowPrivateAccess=true))
	bool bCrouching;

	// 조준에 따라 총기반동 정도 조절
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess=true))
	float RecoilWeight;

	// 캐릭터 회전여부
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess=true))
	bool bTurningInPlace;
	
};
