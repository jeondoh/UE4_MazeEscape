// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

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

	/**************************************************************************************************/
	
// Getter & Setter	
public:
	FORCEINLINE int32 GetAmmo() const {return Ammo;}
	
};
