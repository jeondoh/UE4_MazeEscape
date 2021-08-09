// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy.h"
#include "BossSpawn.h"

// Sets default values
ABossSpawn::ABossSpawn()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ABossSpawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABossSpawn::SpawnBossEnemy(bool bSpawn)
{
	if(bSpawn)
	{
		FActorSpawnParameters Param;
		Param.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AEnemy>(BossSpawnClass,
			GetActorLocation(),
			GetActorRotation(), Param);	
	}
}



