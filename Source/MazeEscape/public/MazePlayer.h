// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Character.h"
#include "MazePlayer.generated.h"

UCLASS()
class MAZEESCAPE_API AMazePlayer : public ACharacter
{
	GENERATED_BODY()
	
// 변수선언
public:
	// Sets default values for this character's properties
	AMazePlayer();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

// 함수선언
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 입력 Bind
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/**************************************************************************************************/
	/** 캐릭터 카메라 **/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess=true))
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess=true))
	class UCameraComponent* FollowCamera;
	/**************************************************************************************************/
	/** 캐릭터 이동 **/
	// 좌우회전 (키보드 왼쪽/오른쪽 키)
	float BaseTurnRate;
	// 상하회전 (키보드 위/아래 키)
	float BaseLookUpRate; 
	// 앞뒤이동 (키보드 W/S 키)
	void MoveForward(float Value);
	// 좌우이동 (키보드 A/D 키)
	void MoveRight(float Value);
	/*
	* Rate만큼 좌우회전
	* @param Rate : 정규화된 비율 / Rate = 1 = 회전율 100%
	*/
	void TurnAtRate(float Rate);
	/*
	* Rate만큼 상하회전
	* @param Rate : 정규화된 비율 / Rate = 1 = 회전율 100%
	*/
	void LookUpAtRate(float Rate);
	/**************************************************************************************************/
	
	
// Getter & Setter
public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const {return CameraBoom;}
	FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera;}
};
