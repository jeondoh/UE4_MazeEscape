// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Engine/DataTable.h"
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
// 아이템 타입(탄약/무기 구분)
UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_Etc UMETA(DisplayName = "Etc"),
	EIT_Ammo UMETA(DisplayName = "Ammo"),
	EIT_Weapon UMETA(DisplayName = "Weapon"),
	
	EIT_MAX UMETA(DisplayName = "DefaultMAX")
};
// 데이터 테이블 정의 / 아이템 희귀도
USTRUCT(BlueprintType)
struct FItemRarityTable : public FTableRowBase
{
	GENERATED_BODY()

	// 아이템 테두리(발광)색
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor;
	// 아이템 Widget의 상단 배경색
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightColor;
	// 아이템 Widget의 하단 배경색
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DarkColor;
	// 아이템 별 개수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfStars;
	// 인벤토리 아이템 배경 이미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IconBackground;
	// 무기의 CustomDepth값
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CustomDepthStencil;
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

	void StartItemCurve(class AMazePlayer* SetPlayer, bool bForcePlaySound = false);
	void PlayEquipSound(bool bForcePlaySound = false);

	/* 아이템 테두리 Glow 효과 파라메터 */
	virtual void EnableCustomDepth();
	virtual void DisableCustomDepth();
	void EnableGlowMaterial();
	void DisableGlowMaterial();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	// 컴포넌트 충돌
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 상태에 따라 아이템 속성(자식 클래스에서도 사용됨)
	virtual void SetItemProperties(EItemState State);
	// 간격을 주어서 Item에 반짝이는 효과(glow effect)
	void UpdatePulse();
	// StartPulseTimer 호출
	UFUNCTION()
	void ResetPulseTimer();
	// 타이머 경과 > 경과시간은 UpdatePulse에서 사용
	UFUNCTION()
	void StartPulseTimer();

private:

	// 변수 초기화
	void InitalizedData();
	
	/**************************************************************************************************/
	/** 컴포넌트 **/
	
	// 충돌상자
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ItemData|Properties", meta=(AllowPrivateAccess=true))
	class UBoxComponent* CollisionBox;

	// 아이템 뼈대
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|Properties", meta=(AllowPrivateAccess=true))
	class USkeletalMeshComponent* ItemMesh;

	// 아이템 위젯 범위 Collision
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ItemData|Properties", meta=(AllowPrivateAccess=true))
	class USphereComponent* AreaSphere;

	/**************************************************************************************************/
	/** 위젯 **/

	// 플레이어가 아이템 볼때 나타나는 팝업 위젯
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ItemData|Properties", meta=(AllowPrivateAccess=true))
	class UWidgetComponent* PickupWidget;

	/**************************************************************************************************/
	/** 아이템 정보 **/

	// 아이템 위젯 > 아이템명
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ItemData|Info", meta=(AllowPrivateAccess=true))
	FString ItemName;

	// 탄약수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ItemData|Info", meta=(AllowPrivateAccess=true))
	int32 ItemCount;

	// 아이템 등급
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ItemData|Info", meta=(AllowPrivateAccess=true))
	EItemRarity ItemRarity;

	// 아이템 등급별 Star개수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|Info", meta=(AllowPrivateAccess=true))
	TArray<bool> ActiveStars;

	// 등급별 Star 세팅
	void SetSwtichStars();

	// 등급별 ActiveStars배열 초기화
	UFUNCTION()
	void SetActiveStars(int8 Num);

	/**************************************************************************************************/
	/* 아이템 상태 */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|State", meta=(AllowPrivateAccess=true))
	EItemState ItemState;

	// 아이템 회전 효과
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ItemData|State", meta=(AllowPrivateAccess=true))
	bool bRotate;
	// 아이템 회전 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ItemData|State", meta=(AllowPrivateAccess=true))
	float RotateSpeed;

	void ItemRotate(float DeltaTime);

	/**************************************************************************************************/
	/* 아이템 획득 */
	
	// 아이템 획득시 아이템 z위치
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ItemData|Interp", meta=(AllowPrivateAccess=true))
	class UCurveFloat* ItemZCurve;

	// Interp 시작위치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|Interp", meta=(AllowPrivateAccess=true))
	FVector ItemInterpStartLocation;

	// 카메라 앞 interp 타겟 위치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|Interp", meta=(AllowPrivateAccess=true))
	FVector CameraTargetLocation;

	// Interp 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|Interp", meta=(AllowPrivateAccess=true))
	bool bInterping;

	// interp 타이머
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|Interp", meta=(AllowPrivateAccess=true))
	FTimerHandle ItemInerpTimer;

	// 플레이어
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|Interp", meta=(AllowPrivateAccess=true))
	class AMazePlayer* InterpPlayer;

	// 엔진 -> 커브에셋 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ItemData|Interp", meta=(AllowPrivateAccess=true))
	float ZCurveTime;

	void FinishInterping();

	// EquipInterping 상태일때 커브를 이용한 아이템 획득
	void ItemInterp(float DeltaTime);

	float InterpInitalYawOffset;

	// 커브 > 아이템 크기 조정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ItemData|Interp", meta=(AllowPrivateAccess=true))
	UCurveFloat* ItemScaleCurve;

	// 아이템 줍는 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ItemData|Sound", meta=(AllowPrivateAccess=true))
	class USoundCue* PickupSound;

	void PlayPickupSound(bool bForcePlaySound = false);
	
	// 아이템 장착 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ItemData|Sound", meta=(AllowPrivateAccess=true))
	class USoundCue* EquipSound;

	/**************************************************************************************************/
	/* 아이템 타입 */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ItemData|Type", meta=(AllowPrivateAccess=true))
	EItemType ItemType;
	// Interp가 위치할 Player의 InterpLocations Index
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|Type", meta=(AllowPrivateAccess=true))
	int32 InterpLocIndex;
	
	FVector GetInterpLocation();

	/**************************************************************************************************/
	/* 아이템 테두리 Glow 효과 파라메터 */

	// 아이템 메테리얼 Index
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ItemData|Mat", meta=(AllowPrivateAccess=true))
	int32 MaterialIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|Mat", meta=(AllowPrivateAccess=true))
	UMaterialInstanceDynamic* DynamicMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ItemData|Mat", meta=(AllowPrivateAccess=true))
	UMaterialInstance* MaterialInstance;

	bool bCanChangeCustomDepth;

	// Curve > Dynamic Material 파라메터값
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ItemData|Mat|Curve", meta=(AllowPrivateAccess=true))
	class UCurveVector* PulseCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ItemData|Mat|Curve", meta=(AllowPrivateAccess=true))
	UCurveVector* InterpPulseCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ItemData|Mat|Curve", meta=(AllowPrivateAccess=true))
	FTimerHandle PulseTimer;
	// PulseTimer 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ItemData|Mat|Curve", meta=(AllowPrivateAccess=true))
	float PulseCurveTime;

	// 밝기
	UPROPERTY(VisibleAnywhere, Category="ItemData|Mat|Curve", meta=(AllowPrivateAccess=true))
	float GlowAmount;

	UPROPERTY(VisibleAnywhere, Category="ItemData|Mat|Curve", meta=(AllowPrivateAccess=true))
	float FresnelExponent;

	UPROPERTY(VisibleAnywhere, Category="ItemData|Mat|Curve", meta=(AllowPrivateAccess=true))
	float FresnelReflectFraction;

	virtual void InitializeCustomDepth();

	/**************************************************************************************************/
	/* 인벤토리 */

	// 인벤토리 아이템 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ItemData|Inventory", meta=(AllowPrivateAccess=true))
	UTexture2D* IconItem;
	// 인벤토리 탄약 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ItemData|Inventory", meta=(AllowPrivateAccess=true))
	UTexture2D* AmmoItem;
	// 인벤토리 배열 Index
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|Inventory", meta=(AllowPrivateAccess=true))
	int32 SlotIndex;
	// 인벤토리 공간여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|Inventory", meta=(AllowPrivateAccess=true))
	bool bCharacterInventoryFull;
	
	/**************************************************************************************************/

	// 아이템 희귀도 DataTable
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="DataTable", meta=(AllowPrivateAccess=true))
	class UDataTable* ItemRarityDataTable;
	// 아이템 테두리(발광)색
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|DataTable|Rarity", meta=(AllowPrivateAccess=true))
	FLinearColor GlowColor;
	// 아이템 Widget의 상단 배경색
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|DataTable|Rarity", meta=(AllowPrivateAccess=true))
	FLinearColor LightColor;
	// 아이템 Widget의 하단 배경색
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|DataTable|Rarity", meta=(AllowPrivateAccess=true))
	FLinearColor DarkColor;
	// 아이템 별 개수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|DataTable|Rarity", meta=(AllowPrivateAccess=true))
	int32 NumberOfStars;
	// 인벤토리 아이템 배경 이미지
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ItemData|DataTable|Rarity", meta=(AllowPrivateAccess=true))
	UTexture2D* IconBackground;

	void SetRarityDataTable();

	/**************************************************************************************************/
	
// Getter & Setter
public:
	FORCEINLINE UWidgetComponent* GetPickupWidget() const {return PickupWidget;}
	
	FORCEINLINE USphereComponent* GetAreaSphere() const {return AreaSphere;}
	
	FORCEINLINE UBoxComponent* GetCollisionBox() const {return CollisionBox;}
	
	FORCEINLINE EItemState GetItemState() const {return ItemState;}
	void SetItemState(EItemState State);
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const {return ItemMesh;}
	
	FORCEINLINE USoundCue* GetEquipSound() const {return EquipSound;}
	FORCEINLINE void SetEquipSound(USoundCue* Sound) {EquipSound = Sound;}
	
	FORCEINLINE USoundCue* GetPickupSound() const {return PickupSound;}
	FORCEINLINE void SetPickupSound(USoundCue* Sound) {PickupSound = Sound;}
	
	FORCEINLINE void SetItemName(FString Name) {ItemName = Name;}
	
	FORCEINLINE void SetIconItem(UTexture2D* Icon) {IconItem = Icon;}
	
	FORCEINLINE void SetAmmoIcon(UTexture2D* Icon) {AmmoItem = Icon;}
	
	FORCEINLINE int32 GetItemCount() const {return ItemCount;}
	
	FORCEINLINE int32 GetSlotIndex() const {return SlotIndex;}
	FORCEINLINE void SetSlotIndex(int32 Index) {SlotIndex = Index;}
	
	FORCEINLINE void SetCharacter(AMazePlayer* Player) {InterpPlayer = Player;}
	FORCEINLINE void SetCharacterInventoryFull(bool Full) {bCharacterInventoryFull = Full;}

	FORCEINLINE FLinearColor GetGlowColor() const {return GlowColor;}

	FORCEINLINE int32 GetMaterialIndex() const {return MaterialIndex;}
	FORCEINLINE void SetMaterialIndex(int32 Index) {MaterialIndex = Index;}
	
	FORCEINLINE UMaterialInstance* GetMaterialInstance() const {return MaterialInstance;}
	FORCEINLINE void SetMaterialInstance(UMaterialInstance* Instance){MaterialInstance = Instance;}
	
	FORCEINLINE UMaterialInstanceDynamic* GetMaterialInstanceDynamic() const {return DynamicMaterialInstance;}
	FORCEINLINE void SetMaterialInstanceDynamic(UMaterialInstanceDynamic* InstanceDynamic){DynamicMaterialInstance = InstanceDynamic;}

};
