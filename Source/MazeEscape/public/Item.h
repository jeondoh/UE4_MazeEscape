// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "Item.generated.h"

// 아이템 등급 ENUM
UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged UMETA(DisplayName = "Damaged"),
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_UnCommon UMETA(DisplayName = "UnCommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Legendary UMETA(DisplayName = "Legendary"),
	
	EIR_MAX UMETA(DisplayName = "DefaultMAX")
};
// 아이템 상태
UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_Pickup UMETA(DisplayName = "Pickup"),
	EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
	EIS_PickedUp UMETA(DisplayName = "PickedUp"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),
	EIS_Falling UMETA(DisplayName = "Falling"),
	
	EIS_MAX UMETA(DisplayName = "DefaultMAX")
};

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

	// 변수 초기화
	void InitalizedData();
	
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
	/** 아이템 정보 **/

	// 아이템 위젯 > 아이템명
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Info", meta=(AllowPrivateAccess=true))
	FString ItemName;

	// 탄약수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Info", meta=(AllowPrivateAccess=true))
	int32 ItemCount;

	// 아이템 등급
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Info", meta=(AllowPrivateAccess=true))
	EItemRarity ItemRarity;

	// 아이템 등급별 Star개수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item|Info", meta=(AllowPrivateAccess=true))
	TArray<bool> ActiveStars;

	// 등급별 Star 세팅
	void SetSwtichStars();

	// 등급별 ActiveStars배열 초기화
	UFUNCTION()
	void SetActiveStars(int8 Num);

	/**************************************************************************************************/
	/* 아이템 상태 */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item|State", meta=(AllowPrivateAccess=true))
	EItemState ItemState;

	// 상태에 따라 아이템 속성
	void SetItemProperties(EItemState State);
	
	/**************************************************************************************************/
	/* 아이템 획득 */
	
	// 아이템 획득시 아이템 z위치
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|Interp", meta=(AllowPrivateAccess=true))
	class UCurveFloat* ItemZCurve;

	// Interp 시작위치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item|Interp", meta=(AllowPrivateAccess=true))
	FVector ItemInterpStartLocation;

	// 카메라 앞 interp 타겟 위치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item|Interp", meta=(AllowPrivateAccess=true))
	FVector CameraTargetLocation;

	// Interp 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item|Interp", meta=(AllowPrivateAccess=true))
	bool bInterping;

	// interp 타이머
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item|Interp", meta=(AllowPrivateAccess=true))
	FTimerHandle ItemInerpTimer;

	// 플레이어
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item|Interp", meta=(AllowPrivateAccess=true))
	class AMazePlayer* InterpPlayer;

	// 엔진 -> 커브에셋 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|Interp", meta=(AllowPrivateAccess=true))
	float ZCurveTime;

	void FinishInterping();

	// EquipInterping 상태일때 커브를 이용한 아이템 획득
	void ItemInterp(float DeltaTime);

	float ItemInterpX;
	float ItemInterpY;

	float InterpInitalYawOffset;

	// 커브 > 아이템 크기 조정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item|Interp", meta=(AllowPrivateAccess=true))
	UCurveFloat* ItemScaleCurve;
	
// Getter & Setter
public:
	FORCEINLINE UWidgetComponent* GetPickupWidget() const {return PickupWidget;}
	FORCEINLINE USphereComponent* GetAreaSphere() const {return AreaSphere;}
	FORCEINLINE UBoxComponent* GetCollisionBox() const {return CollisionBox;}
	FORCEINLINE EItemState GetItemState() const {return ItemState;}
	void SetItemState(EItemState State);

	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const {return ItemMesh;}

	void StartItemCurve(AMazePlayer* SetPlayer);

};
