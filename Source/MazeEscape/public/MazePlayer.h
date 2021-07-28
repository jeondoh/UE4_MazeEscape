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

	// �Է� Bind
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/**************************************************************************************************/
	/** ĳ���� ī�޶� **/
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess=true))
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess=true))
	class UCameraComponent* FollowCamera;
	
	/**************************************************************************************************/
	/** ĳ���� �̵� **/
	
	// �¿�ȸ�� (Ű���� ����/������ Ű)
	float BaseTurnRate;
	// ����ȸ�� (Ű���� ��/�Ʒ� Ű)
	float BaseLookUpRate; 
	// �յ��̵� (Ű���� W/S Ű)
	void MoveForward(float Value);
	// �¿��̵� (Ű���� A/D Ű)
	void MoveRight(float Value);
	// Rate��ŭ �¿�ȸ�� / Rate = 1 = ȸ���� 100%
	void TurnAtRate(float Rate);
	// Rate��ŭ ����ȸ�� / Rate = 1 = ȸ���� 100%
	void LookUpAtRate(float Rate);
	
	/**************************************************************************************************/
	/** ĳ���� �ൿ **/

	// ����߻� (���콺 ��Ŭ��)
	void FireWeapon();
	// �Ѿ��� ���ؼ�(���ڰ�) �������� �̵�
	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	/**************************************************************************************************/
	/** ȿ�� (����/��ƼŬ) **/

	// �ѼҸ� 10�� ���� ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat | Sounds", meta = (AllowPrivateAccess=true))
	class USoundCue* FireSound;
	// ���̷��� > ���� > BareelSocket �߻� ȿ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat | Particles", meta = (AllowPrivateAccess=true))
	class UParticleSystem* MuzzleFlash;
	// �Ѿ� �浹�� ȿ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat | Particles", meta = (AllowPrivateAccess=true))
	class UParticleSystem* ImpactParticle;
	// �Ѿ� �߻� ȿ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat | Particles", meta = (AllowPrivateAccess=true))
	class UParticleSystem* BeamParticles;
	
	/**************************************************************************************************/
	/** �ִϸ��̼� **/

	// �ѱ� �ݵ� ��Ÿ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat | Animate", meta = (AllowPrivateAccess=true))
	class UAnimMontage* HipFireMontage;
	
	/**************************************************************************************************/
	
// Getter & Setter
public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const {return CameraBoom;}
	FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera;}
};
