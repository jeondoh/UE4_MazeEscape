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
	ECS_Equipping UMETA(DisplayName = "Equipping"),

	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FInterpLocation
{
	GENERATED_BODY()
	// Interp위치에 사용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneComponent;
	// 해당 Scene에 위치할 아이템 개수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemCount;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, CurrentSlotIndex, int32, NewSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHighlightIconDelegate, int32, SlotIndex, bool, bStartAnimation);

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
	// 마우스 우측 버튼 클릭하고 있으면 에이밍 지속
	void StillAiming();

	/**************************************************************************************************/
	/** 아이템 획득 **/
	
	void GetPickupItem(AItem* Item);

	/**************************************************************************************************/
	/** 인벤토리 **/
	
	void HighlightInventorySlot();
	void UnHighlightInventorySlot();
	
private:

	// 변수 초기화
	void InitalizedData();

	/**************************************************************************************************/
	/** 캐릭터 상태 **/

	// 전투상태
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State|Combat", meta=(AllowPrivateAccess=true))
	ECombatState CombatState;
	// 캡슐 컴포넌트 높이 > 현재 크기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State|Height", meta = (AllowPrivateAccess=true))
	float CurrentCapsuleHalfHeight;
	// 캡슐 컴포넌트 높이 > 서있을때 크기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State|Height", meta = (AllowPrivateAccess=true))
	float StandingCapsuleHalfHeight;
	// 캡슐 컴포넌트 높이 > 웅크릴때 크기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State|Height", meta = (AllowPrivateAccess=true))
	float CrouchingCapsuleHalfHeight;
	// 평소 걸음속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State|Speed", meta=(AllowPrivateAccess=true))
	float BaseMovementSpeed;
	// 웅크릴때의 걸음속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State|Speed", meta=(AllowPrivateAccess=true))
	float CrouchMovementSpeed;
	// 평소 바닥 마찰정도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State|Speed", meta=(AllowPrivateAccess=true))
	float BaseGroundFriction;
	// 웅크릴때의 바닥 마찰정도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State|Speed", meta=(AllowPrivateAccess=true))
	float CrouchGroundFriction;

	// 웅크리기/서있기의 캡슐 컴포넌트 크기 변화 Interp
	void InterpCapsuleHalfHeight(float DeltaTime);
	
	/**************************************************************************************************/
	/** 캐릭터 카메라 **/
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess=true))
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess=true))
	class UCameraComponent* FollowCamera;
	// 에이밍하지 않을때 카메라 시야
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta = (AllowPrivateAccess=true))
	float CameraDefaultFOV;
	// 에이밍시 카메라 줌
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta = (AllowPrivateAccess=true))
	float CameraZoomedFOV;

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
	// 점프
	virtual void Jump() override;

	/**************************************************************************************************/
	/** 무기 에이밍(마우스 우클릭) **/

	// 에이밍을 하고 있는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Aim", meta = (AllowPrivateAccess=true))
	bool bAiming;
	// 에이밍을 하고 있는지 여부(내부함수에서 이용)
	bool bAimingBUttonPressed;
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
	// 에이밍
	void Aim();
	// 일반상태
	void StopAiming();

	/**************************************************************************************************/
	/** 조준선(Crosshair) **/

	// 조준선 속도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Corsshairs", meta = (AllowPrivateAccess=true))
	float CrosshairSpreadMultiplier;
	// 조준선 펼침
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Corsshairs", meta = (AllowPrivateAccess=true))
	float CrosshairVelocityFactor;
	// 점프시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Corsshairs", meta = (AllowPrivateAccess=true))
	float CrosshairInAirFactor;
	// 조준시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Corsshairs", meta = (AllowPrivateAccess=true))
	float CrosshairAimFactor;
	// 사격시 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Corsshairs", meta = (AllowPrivateAccess=true))
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
	// 사격 사이 타이머 설정
	FTimerHandle AutoFireTimer;
	// Interaction key 상호작용 키
	void InteractionBtnPressed();
	void InteractionBtnReleased();
	// 총알이 조준선(십자가) 방향으로 이동
	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);
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

	// 웅크리기 여부 (키보드 C키)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Animate", meta = (AllowPrivateAccess=true))
	bool bCrouching;

	// 탄약 Reload 애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Animate", meta = (AllowPrivateAccess=true))
	class UAnimMontage* ReloadMontage;

	// 무기스왑 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Animate", meta = (AllowPrivateAccess=true))
	class UAnimMontage* EquippedMontage;

	void CrouchButtonPressed();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	/**************************************************************************************************/
	/** 위젯  **/

	// 조준선으로 겨냥할때 뷰포트가 활성화 되게 설정
	// Item 클래스의 collisionbox & 조준선 LineTrace를 이용해 충돌 처리
	bool TraceUnderCrosshairs(FHitResult& OutHitReuslt, FVector& OutHitLocation, float Multiply);

	// 캐릭터와 겹치는 아이템 추적
	void TraceForItems();

	/**************************************************************************************************/
	/** 아이템  **/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData", meta = (AllowPrivateAccess=true))
	class AItem* TraceHitItemLastFrame;
	
	// 아이템 추적
	bool bShouldTraceForItems;
	// 오버랩되는 아이템 개수
	int8 OverlappedItemCount;

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
	void EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping = false);

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|Ammo", meta=(AllowPrivateAccess=true))
	TMap<EAmmoType, int32> AmmoMap;

	// 시작 9mm 탄약
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ItemData|Ammo", meta=(AllowPrivateAccess=true))
	int32 Starting9mmAmmo;

	// 시작 AR 탄약
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ItemData|Ammo", meta=(AllowPrivateAccess=true))
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
	// 탄약줍기
	void PickUpAmmo(class AAmmo* Ammo);
	// 탄약 확인
	bool CarryingAmo();
	// 탄창 Transform(위치, 회전, 스케일)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|Ammo", meta = (AllowPrivateAccess=true))
	FTransform ClipTransfrom;
	// Scene Component 재장전 중 캐릭터 손에 탄창 부착
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|Ammo|Scene", meta = (AllowPrivateAccess=true))
	USceneComponent* HandSceneComponent;

	/**************************************************************************************************/
	/* interp시 위치 설정 */

	// 무기 위치 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ItemData|Ammo|Scene", meta = (AllowPrivateAccess=true))
	USceneComponent* WeaponInterpComp;
	// 탄약 위치 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ItemData|Ammo|Scene", meta = (AllowPrivateAccess=true))
	USceneComponent* InterpComp1;
	// 탄약 위치 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ItemData|Ammo|Scene", meta = (AllowPrivateAccess=true))
	USceneComponent* InterpComp2;
	// 탄약 위치 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ItemData|Ammo|Scene", meta = (AllowPrivateAccess=true))
	USceneComponent* InterpComp3;
	// 탄약 위치 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ItemData|Ammo|Scene", meta = (AllowPrivateAccess=true))
	USceneComponent* InterpComp4;
	// 탄약 위치 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ItemData|Ammo|Scene", meta = (AllowPrivateAccess=true))
	USceneComponent* InterpComp5;
	// 탄약 위치 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ItemData|Ammo|Scene", meta = (AllowPrivateAccess=true))
	USceneComponent* InterpComp6;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ItemData|Ammo|Scene", meta = (AllowPrivateAccess=true))
	TArray<FInterpLocation> InterpLocations;

	void InitalizeInterpLocations();
	void SetInterpComponent();

	FTimerHandle PickUpSoundTimer;
	FTimerHandle EquipSoundTimer;
	// 아이템 줍기 사운드 여부
	bool bShouldPlayPickUpSound;
	// 무기 획득 사운드 여부
	bool bShouldPlayEquipSound;
	// 아이템 줍기 사운드 시간 리셋시간(다른 아이템 픽업 전 대기시간)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ItemData|Sound|Timer", meta=(AllowPrivateAccess=true))
	float PickUpSoundResetTime;
	// 무기 획득 사운드 시간 리셋시간(다른 아이템 픽업 전 대기시간)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ItemData|Sound|Timer", meta=(AllowPrivateAccess=true))
	float EquipSoundResetTime;

	UFUNCTION()
	void ResetPickupSoundTimer();
	UFUNCTION()
	void ResetEquipSoundTimer();
	
	/**************************************************************************************************/
    /* 인벤토리 */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|Inventory", meta=(AllowPrivateAccess=true))
	TArray<AItem*> Inventory;
	// 인벤토리 무기 개수
	const int32 INVENTORY_CAPACITY{4};
	// 인벤토리에 슬롯 정보 전송(장비장착할때)
	UPROPERTY(BlueprintAssignable, Category="ItemData|Inventory|Delegates", meta=(AllowPrivateAccess=true))
	FEquipItemDelegate EquipItemDelegate;
	// 인벤토리 빈공간에 하이라이트 애니메이션 효과 지정 역활
	UPROPERTY(BlueprintAssignable, Category="ItemData|Inventory|Delegates", meta=(AllowPrivateAccess=true))
	FHighlightIconDelegate HighlightIconDelegate;
	// 강조 표시(애니메이션)된 슬롯의 인덱스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|Inventory|Delegates", meta=(AllowPrivateAccess=true))
	int32 HighlightedSlot;
	// 빈 인벤토리 인덱스 가져오기
	int32 GetEmptyInventorySlot();
	void SetHighlightInventorySlot();
	
	/**************************************************************************************************/
	/* 스왑 */

	void ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex);
	void OnekeyPressed();
	void TwokeyPressed();
	void ThreekeyPressed();
	void FourkeyPressed();
	void KeyPressedToEquipped(int32 SlotIndex);

	/**************************************************************************************************/

// Getter & Setter
public:
	FORCEINLINE ECombatState GetCombatState() const {return CombatState;}
	FORCEINLINE USpringArmComponent* GetCameraBoom() const {return CameraBoom;}
	FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera;}
	FORCEINLINE bool GetAiming() const {return bAiming;}
	FORCEINLINE bool GetCrouching() const {return bCrouching;}
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetCrosshairSpreadMultiplier() const {return CrosshairSpreadMultiplier;}

	FORCEINLINE int8 GetOverlappedItemCount() const {return OverlappedItemCount;}
	// 오버랩되는 아이템 개수 계산 
	// 오버랩시 아이템 위젯 창 팝업
	void IncrementOverlappedItemCount(int8 Amount);

	FInterpLocation GetInterpLocation(int32 Index);
	// ItemCount가 가장 적은 interpLocations의 Index 반환
	int32 GetInterpLocationIndex();

	void IncrementInterpLocItemCount(int32 Index, int32 Amount);

	FORCEINLINE bool GetShouldPlayPickUpSound() const {return bShouldPlayPickUpSound;}
	FORCEINLINE bool GetShouldPlayEquipSound() const {return bShouldPlayEquipSound;}

	void StartPickUpSoundTimer();
	void StartEquipSoundTimer();
	
	FORCEINLINE AWeapon* GetEquippedWeapon() const {return EquippedWeapon;}
};
