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
	/* 아이템 테두리 Glow 효과 파라메터 */
	virtual void EnableCustomDepth() override;
	virtual void DisableCustomDepth() override;

protected:
	virtual void BeginPlay() override;

	// Item 클래스의 함수 재정의
	virtual void SetItemProperties(EItemState State) override;

private:
	/**************************************************************************************************/
	/** 컴포넌트 **/
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ammo|Component", meta=(AllowPrivateAccess=true))
	UStaticMeshComponent* AmmoMesh;

	// 오버랩시 탄약 자동줍기 위한 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ammo|Component", meta=(AllowPrivateAccess=true))
	class USphereComponent* AmmoCollisionSphere;

	// 컴포넌트 충돌
	UFUNCTION()
	void AmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	/**************************************************************************************************/
	/* 상태 */

	// 탄약상태
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ammo|State", meta=(AllowPrivateAccess=true))
	EAmmoType AmmoType;

	/**************************************************************************************************/
	/* 위젯 */

	// 탄약 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ammo|Widget", meta=(AllowPrivateAccess=true))
	UTexture2D* AmmoIconTexture;
	
// Getter & Setter
public:
	FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const {return AmmoMesh;}
	FORCEINLINE EAmmoType GetAmmoType() const {return AmmoType;}
	
};
