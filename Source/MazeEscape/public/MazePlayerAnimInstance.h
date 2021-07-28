// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MazePlayerAnimInstance.generated.h"

/**
* 
*/
UCLASS()
class MAZEESCAPE_API UMazePlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);
	
	virtual void NativeInitializeAnimation() override;

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
};
