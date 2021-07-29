// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UCLASS()
class MAZEESCAPE_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 컴포넌트 충돌
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	/**************************************************************************************************/
	/** 컴포넌트 **/
	
	// 충돌상자
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Properties", meta=(AllowPrivateAccess=true))
	class UBoxComponent* CollisionBox;

	// 아이템 뼈대
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item|Properties", meta=(AllowPrivateAccess=true))
	class USkeletalMeshComponent* ItemMesh;

	// 아이템 위젯 범위 Collision
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Properties", meta=(AllowPrivateAccess=true))
	class USphereComponent* AreaSphere;
	
	/**************************************************************************************************/
	/** 위젯 **/

	// 플레이어가 아이템 볼때 나타나는 팝업 위젯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Properties", meta=(AllowPrivateAccess=true))
	class UWidgetComponent* PickupWidget;

	/**************************************************************************************************/
	
// Getter & Setter
public:
	FORCEINLINE UWidgetComponent* GetPickupWidget() const {return PickupWidget;}

};
