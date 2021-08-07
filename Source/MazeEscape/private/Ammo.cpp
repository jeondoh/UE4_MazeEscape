// Fill out your copyright notice in the Description page of Project Settings.


#include "Ammo.h"

#include "MazePlayer.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AAmmo::AAmmo()
{
	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
	SetRootComponent(AmmoMesh);

	GetCollisionBox()->SetupAttachment(GetRootComponent());
	GetPickupWidget()->SetupAttachment(GetRootComponent());
	GetAreaSphere()->SetupAttachment(GetRootComponent());

	AmmoCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AmmoCollisionSphere"));
	AmmoCollisionSphere->SetupAttachment(GetRootComponent());
	AmmoCollisionSphere->SetSphereRadius(50.f);
}

void AAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAmmo::BeginPlay()
{
	Super::BeginPlay();

	AmmoCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAmmo::AmmoSphereOverlap); 
}

void AAmmo::SetItemProperties(EItemState State)
{
	Super::SetItemProperties(State);
	
	switch(State)
    {
    case EItemState::EIS_Pickup:
    	// Mesh Properties 설정
    	AmmoMesh->SetSimulatePhysics(false);
    	AmmoMesh->SetEnableGravity(false);
    	AmmoMesh->SetVisibility(true);
    	AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    	AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    	break;
    case EItemState::EIS_Equipped:
    	// Mesh Properties 설정
    	AmmoMesh->SetSimulatePhysics(false);
    	AmmoMesh->SetEnableGravity(false);
    	AmmoMesh->SetVisibility(true);
    	AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    	AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    	break;
    case EItemState::EIS_Falling:
    	// Mesh Properties 설정
    	AmmoMesh->SetSimulatePhysics(true);
    	AmmoMesh->SetEnableGravity(true);
    	AmmoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    	AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    	AmmoMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
    	break;
    case EItemState::EIS_EquipInterping:
    	// Mesh Properties 설정
        AmmoMesh->SetSimulatePhysics(false);
        AmmoMesh->SetEnableGravity(false);
        AmmoMesh->SetVisibility(true);
        AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
        AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    	break;	
    default:
		;
    }
}

void AAmmo::AmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor)
	{
		AMazePlayer* Player = Cast<AMazePlayer>(OtherActor);
		if(Player)
		{
			StartItemCurve(Player);
			AmmoCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AAmmo::EnableCustomDepth()
{
	AmmoMesh->SetRenderCustomDepth(true);
}

void AAmmo::DisableCustomDepth()
{
	AmmoMesh->SetRenderCustomDepth(false);
}
