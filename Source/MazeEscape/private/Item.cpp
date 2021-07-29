// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"

#include "MazePlayer.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 변수 초기화
	InitalizedData();
	
	// 컴포넌트 설정
	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ITemMesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(GetRootComponent());

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	// PickupWidget 숨기기
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(false);		
	}
	// 등급별 Star 세팅
	SetSwtichStars();

	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItem::InitalizedData()
{
	ItemName = FString("Default"); // 아이템명
	ItemCount = 0; // 탄약수
	ItemRarity = EItemRarity::EIR_Common; // 아이템 등급
}

void AItem::SetSwtichStars()
{
	// 배열 초기화
	for(int8 i = 0; i <= 5; i++)
	{
		ActiveStars.Add(false);
	}
	
	// 등급별 Star 세팅
	switch(ItemRarity)
	{
		case EItemRarity::EIR_Damaged:
			SetActiveStars(1);
			break;
		case EItemRarity::EIR_Common:
			SetActiveStars(2);
			break;
		case EItemRarity::EIR_UnCommon:
			SetActiveStars(3);
			break;
		case EItemRarity::EIR_Rare:
			SetActiveStars(4);
			break;
		case EItemRarity::EIR_Legendary:
			SetActiveStars(5);
			break;
		default:
			;
	}
}

void AItem::SetActiveStars(int8 Num)
{
	for(int8 i = 1; i <= Num; i++)
	{
		ActiveStars[i] = true;
	}
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor)
	{
		AMazePlayer* Player = Cast<AMazePlayer>(OtherActor);
		if(Player)
		{
			Player->IncrementOverlappedItemCount(1);
		}
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(OtherActor)
	{
		AMazePlayer* Player = Cast<AMazePlayer>(OtherActor);
		if(Player)
		{
			Player->IncrementOverlappedItemCount(-1);
		}
	}
}

