// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Weapon.h"
#include "GameFramework/Character.h"
#include "AmmoType.h"
#include "MazePlayer.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};

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

	/**************************************************************************************************/
	/** 아이템 획득 **/

	// 아이템의 위치를 카메라 위치의 CameraInterpDistance만큼 앞으로, CameraInterpElevation만큼 위로 배치
	FVector GetCameraInterpLocation();

	void GetPickupItem(AItem* Item);
	
	/**************************************************************************************************/

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
	// 마우스 X 움직임
	void Turn(float Value);
	// 마우스 Y 움직임
	void LookUp(float Value);

	/**************************************************************************************************/
	/** 무기 에이밍(마우스 우클릭) **/

	// 에이밍하지 않을때 카메라 시야
	float CameraDefaultFOV;
	
	// 에이밍을 하고 있는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Aim", meta = (AllowPrivateAccess=true))
	bool bAiming;

	// 에이밍 하지 않을때 좌우회전
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Aim", meta = (AllowPrivateAccess=true))
	float HipTurnRate;
	
	// 에이밍 하지 않을때 상하회전
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Aim", meta = (AllowPrivateAccess=true))
	float HipLookUpRate;

	// 에이밍시 좌우회전
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Aim", meta = (AllowPrivateAccess=true))
	float AimingTurnRate;

	// 에이밍시 상하회전
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Aim", meta = (AllowPrivateAccess=true))
	float AimingLookupRate;
	
	// 에이밍시 카메라 줌
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat|Aim", meta = (AllowPrivateAccess=true))
	float CameraZoomedFOV;

	// 카메라 현재 위치
	float CameraCurrentFOV;

	// 에이밍 확대/축소 Interp속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Aim", meta = (AllowPrivateAccess=true))
	float ZoomInterpSpeed;

	// 조준하지 않을때의 마우스 좌우 감도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Aim|Mouse",
		meta = (AllowPrivateAccess=true), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;

	// 조준하지 않을때의 마우스 상하 감도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Aim|Mouse",
		meta = (AllowPrivateAccess=true) , meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	// 조준할때의 마우스 좌우 감도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Aim|Mouse",
		meta = (AllowPrivateAccess=true) , meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;

	// 조준할때의 마우스 상하 감도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Aim|Mouse",
		meta = (AllowPrivateAccess=true) , meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;
	
	// 에이밍 (마우스 오른쪽 버튼 Press)
	void AimingButtonPressed();

	// 에이밍 (마우스 오른쪽 버튼 No Press)
	void AimingButtonReleased();

	// 카메라 줌인/줌아웃 Interp 사용해 부드럽게 이동 (마우스 오른쪽 버튼)
	void CameraInterpZoom(float DeltaTime);

	// 에이밍 회전 변수 변경
	void SetLookRate();

	/**************************************************************************************************/
	/** 조준선(Crosshair) **/

	// 조준선 속도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Corsshairs", meta = (AllowPrivateAccess=true))
	float CrosshairSpreadMultiplier;

	// 조준선 펼침
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Corsshairs", meta = (AllowPrivateAccess=true))
	float CrosshairVelocityFactor;

	// 점프시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Corsshairs", meta = (AllowPrivateAccess=true))
	float CrosshairInAirFactor;

	// 조준시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Corsshairs", meta = (AllowPrivateAccess=true))
	float CrosshairAimFactor;

	// 사격시 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Corsshairs", meta = (AllowPrivateAccess=true))
	float CorsshairShootingFactor;

	// 조준선 계산
	void CalculateCrosshairSpread(float DeltaTime);

	// 사격후 시간
	float ShootTimeDuration;
	// 사격 여부
	bool bFiringBullet;
	// 사격 타이머 
	FTimerHandle CrosshairShootTimer;
	// 사격 후 조준선 벌어지게
	void StartCrosshairBulletFire();
	// Timer 이후 조준선 복구
	UFUNCTION()
	void FinishCrosshairBulletFire();
	// 뷰포트 크기 & 조준선 위치
	void GetViewPortCrossHair(FVector2D& ViewportSize, FVector& CrossHairWorldPosition, FVector& CrossHairWorldDirection, bool& bScreenToWorld);
	
	/**************************************************************************************************/
	/** 캐릭터 행동 **/

	// 왼쪽 마우스 클릭 여부
	bool bFireButtonPressed;
	// 총 발사 여부
	// bool bShouldFire;
	// 자동발사 사격속도(간격)
	float AutomaticFireRate;
	// 사격 사이 타이머 설정
	FTimerHandle AutoFireTimer;
	// Interaction key 상호작용 키
	void InteractionBtnPressed();
	void InteractionBtnRelease();
	// 총알이 조준선(십자가) 방향으로 이동
	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);
	// 전투상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Stat|", meta=(AllowPrivateAccess=true))
	ECombatState CombatState;
	// 무기발사 (마우스 좌클릭)
	void FireWeapon();
	// 사격 소리
	void PlayFireSound();
	// 총알
	void SendBullet();
	// 사격 몽타주 애니메이션
	void PlayGunFireMontage();
	// 사격
	void FireButtonPressed();
	// 비사격
	void FireButtonReleased();
	// 사격 타이머
	void StartFireTimer();
	// 자동발사 재설정
	UFUNCTION()
	void AutoFireReset();
	
	/**************************************************************************************************/
	/** 효과 (사운드/파티클) **/

	// 총소리 10개 랜덤 재생
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Sounds", meta = (AllowPrivateAccess=true))
	class USoundCue* FireSound;
	// 스켈레톤 > 무기 > BareelSocket 발사 효과
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Particles", meta = (AllowPrivateAccess=true))
	class UParticleSystem* MuzzleFlash;
	// 총알 충돌시 효과
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Particles", meta = (AllowPrivateAccess=true))
	class UParticleSystem* ImpactParticle;
	// 총알 발사 효과
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Particles", meta = (AllowPrivateAccess=true))
	class UParticleSystem* BeamParticles;
	
	/**************************************************************************************************/
	/** 애니메이션 **/

	// 총기 반동 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Animate", meta = (AllowPrivateAccess=true))
	class UAnimMontage* HipFireMontage;

	/**************************************************************************************************/
	/** 위젯  **/

	// 조준선으로 겨냥할때 뷰포트가 활성화 되게 설정
	// Item 클래스의 collisionbox & 조준선 LineTrace를 이용해 충돌 처리
	bool TraceUnderCrosshairs(FHitResult& OutHitReuslt, FVector& OutHitLocation, float Multiply);

	// 캐릭터와 겹치는 아이템 추적
	void TraceForItems();

	/**************************************************************************************************/
	/** 아이템  **/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Items", meta = (AllowPrivateAccess=true))
	class AItem* TraceHitItemLastFrame;
	
	// 아이템 추적
	bool bShouldTraceForItems;
	// 오버랩되는 아이템 개수
	int8 OverlappedItemCount;

	/**************************************************************************************************/
	/** 아이템 획득 **/

	// Inerp 대상에 대해 카메라에서 앞쪽으로 거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Items", meta=(AllowPrivateAccess=true))
	float CameraInterpDistance;

	// Inerp 대상에 대해 카메라에서 위쪽으로 거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Items", meta=(AllowPrivateAccess=true))
	float CameraInterpElevation;

	/**************************************************************************************************/
	/** 무기 **/

	// 블루프린트에서 기본 무기 지정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat", meta=(AllowPrivateAccess=true))
	TSubclassOf<AWeapon> DefaultWeaponClass;
	
	// 장착된 무기
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta=(AllowPrivateAccess=true))
	AWeapon* EquippedWeapon;

	// 기본무기 장착
	AWeapon* SpawnDefaultWeapon();

	// 무기 장착
	void EquipWeapon(AWeapon* WeaponToEquip);

	// 무기 장착 해제 이후 땅에 떨어트림
	void DropWeapon();

	// 현재 장착된 무기 드롭 & 무기 장착
	void SwapWeapon(AWeapon* WeaponToSwap);
	
	// TraceForItems(캐릭터와 겹치는 아이템 추적 함수) 에서 설정된 아이템 > Null이 될수도 있음 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta=(AllowPrivateAccess=true))
	AItem* TraceHitItem;

	/**************************************************************************************************/
	/** 탄약 **/

	// 탄약 없을때 소리 재생
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ammo|Sounds", meta = (AllowPrivateAccess=true))
	class USoundCue* EmptyBulletSound;
	
	// 탄약 저장 TMAP
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Items|Ammo", meta=(AllowPrivateAccess=true))
	TMap<EAmmoType, int32> AmmoMap;

	// 시작 9mm 탄약
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Items|Ammo", meta=(AllowPrivateAccess=true))
	int32 Starting9mmAmmo;

	// 시작 AR 탄약
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Items|Ammo", meta=(AllowPrivateAccess=true))
	int32 StartingARAmmo;

	// AmmoMap 초기화
	void InitializeAmmoMap();
	// 무기에 탄약이 있는지 확인
	bool WeaponHasAmmo();
	// 재장전 (키보드R키)
	void ReloadButtonPressed();
	// 재장전
	void ReloadWeapon();
	// 애니메이션 Blueprint에서 호출됨 > Reload Finish 노티파이
	UFUNCTION(BlueprintCallable)
	void FinishedReload();
	// 애니메이션 Blueprint에서 호출됨 > GrabClip 노티파이
	UFUNCTION(BlueprintCallable)
	void GrabClip();
	// 애니메이션 Blueprint에서 호출됨 > GrabClip 노티파이
	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	// 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Animate", meta = (AllowPrivateAccess=true))
	class UAnimMontage* ReloadMontage;

	// 탄약 확인
	bool CarryingAmo();

	// 탄창 Transform(위치, 회전, 스케일)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta = (AllowPrivateAccess=true))
	FTransform ClipTransfrom;

	// Scene Component 재장전 중 캐릭터 손에 탄창 부착
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta = (AllowPrivateAccess=true))
	USceneComponent* HandSceneComponent;
		
	/**************************************************************************************************/
	
// Getter & Setter
public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const {return CameraBoom;}
	FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera;}
	FORCEINLINE bool GetAiming() const {return bAiming;}

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetCrosshairSpreadMultiplier() const {return CrosshairSpreadMultiplier;}

	FORCEINLINE int8 GetOverlappedItemCount() const {return OverlappedItemCount;}
	// 오버랩되는 아이템 개수 계산 
	// 오버랩시 아이템 위젯 창 팝업
	void IncrementOverlappedItemCount(int8 Amount);
};
