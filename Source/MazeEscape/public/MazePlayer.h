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

	// 변수 초기화
	void InitalizedData();
	
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
	// Rate만큼 좌우회전 / Rate = 1 = 회전율 100%
	void TurnAtRate(float Rate);
	// Rate만큼 상하회전 / Rate = 1 = 회전율 100%
	void LookUpAtRate(float Rate);

	/**************************************************************************************************/
	/** 캐릭터 무기 조준선(Crosshair) **/

	// 에이밍하지 않을때 카메라 시야
	float CameraDefaultFOV;
	
	// 에이밍을 하고 있는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat | Aim", meta = (AllowPrivateAccess=true))
	bool bAiming;

	// 에이밍 시 카메라 줌
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat | Aim", meta = (AllowPrivateAccess=true))
	float CameraZoomedFOV;

	// 카메라 현재 위치
	float CameraCurrentFOV;

	// 에이밍 확대/축소 Interp속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat | Aim", meta = (AllowPrivateAccess=true))
	float ZoomInterpSpeed;

	// 에이밍 (마우스 오른쪽 버튼 Press)
	void AimingButtonPressed();

	// 에이밍 (마우스 오른쪽 버튼 No Press)
	void AimingButtonReleased();

	// 카메라 줌인/줌아웃 Interp 사용해 부드럽게 이동 (마우스 오른쪽 버튼)
	void CameraInterpZoom(float DeltaTime);
	
	/**************************************************************************************************/
	/** 캐릭터 행동 **/

	// 무기발사 (마우스 좌클릭)
	void FireWeapon();
	// 총알이 조준선(십자가) 방향으로 이동
	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);
	
	/**************************************************************************************************/
	/** 효과 (사운드/파티클) **/

	// 총소리 10개 랜덤 재생
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat | Sounds", meta = (AllowPrivateAccess=true))
	class USoundCue* FireSound;
	// 스켈레톤 > 무기 > BareelSocket 발사 효과
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat | Particles", meta = (AllowPrivateAccess=true))
	class UParticleSystem* MuzzleFlash;
	// 총알 충돌시 효과
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat | Particles", meta = (AllowPrivateAccess=true))
	class UParticleSystem* ImpactParticle;
	// 총알 발사 효과
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat | Particles", meta = (AllowPrivateAccess=true))
	class UParticleSystem* BeamParticles;
	
	/**************************************************************************************************/
	/** 애니메이션 **/

	// 총기 반동 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat | Animate", meta = (AllowPrivateAccess=true))
	class UAnimMontage* HipFireMontage;
	
	/**************************************************************************************************/
	
// Getter & Setter
public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const {return CameraBoom;}
	FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera;}
	FORCEINLINE bool GetAiming() const {return bAiming;}
};
