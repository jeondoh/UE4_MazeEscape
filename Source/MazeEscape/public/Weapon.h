// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "WeaponType.h"
#include "Weapon.generated.h"

// 데이터 테이블 정의 / 무기 속성
USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()
	// 탄약종류 열거형
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAmmoType AmmoType;
	// 탄약수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WeaponAmmo;
	// 탄창용량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagazingCapacity;
	// 무기 주울때 소리
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundCue* PickupSound;
	// 무기 장착 소리
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* EquipSound;
	// 무기 스켈레톤(외형)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* ItemMesh;
	// 무기 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;
	// 인벤토리 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* InventoryIcon;
	// 인벤토리 탄약 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* AmmoIcon;
	// 무기종류별 메테리얼
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* MaterialInstance;
	// 메테리얼 인덱스
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaterialIndex;
	// 탄창 스켈레톤 이름(재장전 탄창 분리시에 필요)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ClipBoneName;
	// 재장전 몽타주 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReloadMontageSection;
	// 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimBP;
	// 조준선 중앙
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsMiddle;
	// 조준선 왼쪽
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsLeft;
	// 조준선 오른쪽
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsRight;
	// 조준선 아래
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsBottom;
	// 조준선 위
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsTop;
	// 자동발사 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoFireRate;
	// 사격시 효과
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UParticleSystem* MuzzleFlash;
	// 사격 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* FireSound;
};

/**
 * 
 */
UCLASS()
class MAZEESCAPE_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	AWeapon();

	virtual void Tick(float DeltaSeconds) override;

	// 무기 던지기
	void ThrowWeapon();
	// 탄약 감소 (MazePlayer::FireWeapon)
	void DecrementAmmo();
	// 탄약 리로드
	void ReloadAmmo(int32 Amount);
	// 탄창용량확인
	bool ClipIsFull();
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	
private:
	/**************************************************************************************************/
	/* 무기 버리기 */
	bool bFalling;
	float ThrowWeaponTime;
	FTimerHandle ThrowWeaponTimer;
	
	void StopFalling();

	/**************************************************************************************************/
	/* 탄약 */

	/** 무기의 탄약수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Ammo", meta=(AllowPrivateAccess=true))
	int32 Ammo;

	// 탄창용량(빈탄창)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Ammo", meta=(AllowPrivateAccess=true))
	int32 MagazineCapacity;
	
	// 탄약(ENUM)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Ammo", meta=(AllowPrivateAccess=true))
	EAmmoType AmmoType;

	// 리로드시 탄창 분리여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon|Properties", meta=(AllowPrivateAccess=true))
	bool bMovingClip;

	// 탄창명(총 고유 탄창명) / 스켈레톤 > 탄창(뼈) 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Properties", meta=(AllowPrivateAccess=true))
	FName ClipBoneName;
	/**************************************************************************************************/
	/* 무기 */
	
	// 무기 타입(ENUM)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Ammo", meta=(AllowPrivateAccess=true))
	EWeaponType WeaponType;

	// 리로드 몽타주 색션 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Ammo", meta=(AllowPrivateAccess=true))
	FName ReloadMontageSection;
	
	/**************************************************************************************************/
	/* 무기 데이터테이블 */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ItemData|DataTable", meta=(AllowPrivateAccess=true))
	UDataTable* WeaponDataTable;

	// 이전 메테리얼 인덱스값
	int32 PreviousMaterialIndex;

	// 조준선 중앙
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|DataTable", meta=(AllowPrivateAccess=true))
	UTexture2D* CrosshairsMiddle;
	// 조준선 왼쪽
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|DataTable", meta=(AllowPrivateAccess=true))
	UTexture2D* CrosshairsLeft;
	// 조준선 오른쪽
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|DataTable", meta=(AllowPrivateAccess=true))
	UTexture2D* CrosshairsRight;
	// 조준선 아래
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|DataTable", meta=(AllowPrivateAccess=true))
	UTexture2D* CrosshairsBottom;
	// 조준선 위
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|DataTable", meta=(AllowPrivateAccess=true))
	UTexture2D* CrosshairsTop;
	// 사격시 효과
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|DataTable", meta=(AllowPrivateAccess=true))
	class UParticleSystem* MuzzleFlash;
	// 사격 사운드
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|DataTable", meta=(AllowPrivateAccess=true))
	USoundCue* FireSound;
	// 자동발사 속도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|DataTable", meta=(AllowPrivateAccess=true))
	float AutoFireRate;

	void SetWeaponDataTable();

	/**************************************************************************************************/
	
// Getter & Setter	
public:
	FORCEINLINE int32 GetAmmo() const {return Ammo;}
	FORCEINLINE int32 GetMagazineCapacity() const {return MagazineCapacity;}
	FORCEINLINE EWeaponType GetWeaponType() const {return WeaponType;}
	FORCEINLINE EAmmoType GetAmmoType() const {return AmmoType;}

	FORCEINLINE FName GetReloadMontageSection() const {return ReloadMontageSection;}
	FORCEINLINE void SetReloadMontageSection(FName Name) {ReloadMontageSection = Name;}
	
	FORCEINLINE FName GetClipBoneName() const {return ClipBoneName;}
	FORCEINLINE void SetClipBoneName(FName Name) {ClipBoneName = Name;}
	
	FORCEINLINE void SetClipBoneName(bool Move) {bMovingClip = Move;}

	FORCEINLINE float GetAutoFireRate() const {return AutoFireRate;}
	
	FORCEINLINE UParticleSystem* GetMuzzleFlash() const {return MuzzleFlash;}

	FORCEINLINE USoundCue* GetFireSound() const {return FireSound;}
};
