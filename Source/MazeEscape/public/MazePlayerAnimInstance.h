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

	// ĳ���� �̵��ӵ�
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	float Speed;

	// ĳ���Ͱ� ���������� Ȯ��
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bIsInAir;

	// ĳ���Ͱ� �����̰� �ִ��� Ȯ��
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement", meta=(AllowPrivateAccess=true))
	bool bIsAccelerating;

	// ������ YAW > ������ġ & ĳ�����̵���ǥ�� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(AllowPrivateAccess=true))
	float MovementOffsetYaw;

	// ĳ���� �̵��ӵ��� 0�� �ƴ� ������ ������ �� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(AllowPrivateAccess=true))
	float LastMovementOffsetYaw;
};
