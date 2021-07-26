// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MazePlayer.generated.h"

UCLASS()
class MAZEESCAPE_API AMazePlayer : public ACharacter
{
	GENERATED_BODY()
	
// ��������
public:
	// Sets default values for this character's properties
	AMazePlayer();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

// �Լ�����
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/**************************************************************************************************/
	// ĳ���� �� ī�޶� ��ġ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess=true))
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess=true))
	class UCameraComponent* FollowCamera;
	/**************************************************************************************************/
	
// Getter & Setter
public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const {return CameraBoom;}
	FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera;}
};
