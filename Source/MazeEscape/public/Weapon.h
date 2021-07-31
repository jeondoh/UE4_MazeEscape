// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SubmachineGun UMETA(DisplayName = "SubmachineGun"),
	EWT_AssaultRifle UMETA(DisplayName = "AsaultRifle"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
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
protected:

	
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ammo", meta=(AllowPrivateAccess=true))
	int32 Ammo;

	// 탄창용량(빈탄창)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ammo", meta=(AllowPrivateAccess=true))
	int32 MagazineCapacity;
	
	// 탄약(ENUM)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ammo", meta=(AllowPrivateAccess=true))
	EAmmoType AmmoType;

	/**************************************************************************************************/
	/* 무기 */
	
	// 무기 타입(ENUM)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ammo", meta=(AllowPrivateAccess=true))
	EWeaponType WeaponType;

	// 리로드 몽타주 색션 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ammo", meta=(AllowPrivateAccess=true))
	FName ReloadMontageSection;
	
// Getter & Setter	
public:
	FORCEINLINE int32 GetAmmo() const {return Ammo;}
	FORCEINLINE int32 GetMagazineCapacity() const {return MagazineCapacity;}
	FORCEINLINE EWeaponType GetWeaponType() const {return WeaponType;}
	FORCEINLINE EAmmoType GetAmmoType() const {return AmmoType;}
	FORCEINLINE FName GetReloadMontageSection() const {return ReloadMontageSection;}
};
