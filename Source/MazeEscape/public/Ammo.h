// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Ammo.generated.h"

/**
 * 
 */
UCLASS()
class MAZEESCAPE_API AAmmo : public AItem
{
	GENERATED_BODY()

public:
	AAmmo();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	// Item 클래스의 함수 재정의
	virtual void SetItemProperties(EItemState State) override;

private:
	/**************************************************************************************************/
	/** 컴포넌트 **/
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ammo|Component", meta=(AllowPrivateAccess=true))
	UStaticMeshComponent* AmmoMesh;
	/**************************************************************************************************/
	/* 상태 */

	// 탄약상태
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ammo|State", meta=(AllowPrivateAccess=true))
	EAmmoType AmmoType;
	
// Getter & Setter
public:
	FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const {return AmmoMesh;}
	FORCEINLINE EAmmoType GetAmmoType() const {return AmmoType;}
	
};
