// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Kismet/GameplayStatics.h"

AWeapon::AWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ThrowWeaponTime = 0.7f;
	bFalling = false;
	Ammo = 30;
	MagazineCapacity = 30;
	WeaponType = EWeaponType::EWT_SubmachineGun;
	AmmoType = EAmmoType::EAT_9mm;
	ReloadMontageSection = FName(TEXT("Reload SMG"));
	ClipBoneName = FName(TEXT("smg_clip"));
}

void AWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 무기 세우기
	if(GetItemState() == EItemState::EIS_Falling && bFalling)
	{
		const FRotator MeshRotation{0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f};
		GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	} 
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	// 무기 테이블 로드
	SetWeaponDataTable();
}

void AWeapon::ThrowWeapon()
{
	FRotator MeshRotation{0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f};
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector MeshForward{GetItemMesh()->GetForwardVector()};
	const FVector MeshRight{GetItemMesh()->GetRightVector()};
	// 무기 던지는 방향
	// RotateAngleAxis = 축(2)을 중심으로 회전(1)
	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);

	float RandomRotation = FMath::FRandRange(10.f, 30.f);
	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
	ImpulseDirection *= 20000.f;
	GetItemMesh()->AddImpulse(ImpulseDirection); // 충격 가함

	bFalling = true;
	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);
}

void AWeapon::DecrementAmmo()
{
	if(Ammo < 0)
	{
		Ammo = 0;
	}
	--Ammo;
}

void AWeapon::StopFalling()
{
	bFalling = false;
	SetItemState(EItemState::EIS_Pickup);
	EnableGlowMaterial();
	// 타이머 경과 > 경과시간은 UpdatePulse에서 사용
	StartPulseTimer();
}

void AWeapon::SetWeaponDataTable()
{
	const FString WeaponTablePath{TEXT("DataTable'/Game/00_MazeDev/DataTable/WeaponDataTable.WeaponDataTable'")};
	UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));

	if (WeaponTableObject)
	{
		FWeaponDataTable* WeaponDataRow = nullptr;
		switch (WeaponType)
		{
		case EWeaponType::EWT_SubmachineGun:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("SubmachineGun"), TEXT(""));			
			break;
		case EWeaponType::EWT_AssaultRifle:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("AssaultRifle"), TEXT(""));			
			break;
		}
		if(WeaponDataRow)
		{
			AmmoType = WeaponDataRow->AmmoType;
            Ammo = WeaponDataRow->WeaponAmmo;
            MagazineCapacity = WeaponDataRow->MagazingCapacity;
            SetPickupSound(WeaponDataRow->PickupSound); 
            SetEquipSound(WeaponDataRow->EquipSound); 
            GetItemMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);
			SetItemName(WeaponDataRow->ItemName);
			SetIconItem(WeaponDataRow->InventoryIcon);
			SetAmmoIcon(WeaponDataRow->AmmoIcon);
			SetClipBoneName(WeaponDataRow->ClipBoneName);
			SetMaterialInstance(WeaponDataRow->MaterialInstance);
			SetReloadMontageSection(WeaponDataRow->ReloadMontageSection);
			GetItemMesh()->SetAnimInstanceClass(WeaponDataRow->AnimBP);
			// 메테리얼 초기화 후 적용
			PreviousMaterialIndex = GetMaterialIndex();
			GetItemMesh()->SetMaterial(PreviousMaterialIndex, nullptr);
			SetMaterialIndex(WeaponDataRow->MaterialIndex);
			// 조준선 설정 
			CrosshairsMiddle = WeaponDataRow->CrosshairsMiddle;
            CrosshairsLeft = WeaponDataRow->CrosshairsLeft;
            CrosshairsRight = WeaponDataRow->CrosshairsRight;
            CrosshairsBottom = WeaponDataRow->CrosshairsBottom;
            CrosshairsTop = WeaponDataRow->CrosshairsTop;
			// 사격효과 설정
			MuzzleFlash = WeaponDataRow->MuzzleFlash;
			FireSound = WeaponDataRow->FireSound;
			AutoFireRate = WeaponDataRow->AutoFireRate;
		}
	}
	// GLOW 메테리얼 효과
	if(GetMaterialInstance())
	{
		SetMaterialInstanceDynamic(UMaterialInstanceDynamic::Create(GetMaterialInstance(), this));
		GetMaterialInstanceDynamic()->SetVectorParameterValue(TEXT("FresnelColor"), GetGlowColor());
		GetItemMesh()->SetMaterial(GetMaterialIndex(), GetMaterialInstanceDynamic());
		EnableGlowMaterial();
	}
}

bool AWeapon::ClipIsFull()
{
	return Ammo >= MagazineCapacity;
}

void AWeapon::ReloadAmmo(int32 Amount)
{
	checkf(Ammo + Amount <= MagazineCapacity, TEXT("탄창용량에 초과된 탄약은 Reload 됩니다."));
	Ammo += Amount;
}
