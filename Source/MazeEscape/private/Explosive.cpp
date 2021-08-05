// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
AExplosive::AExplosive()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AExplosive::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AExplosive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AExplosive::BulletHit_Implementation(FHitResult HitResult)
{
	// 사운드
    if(ExplodeSound)
    {
    	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());
    }
    // 파티클
    if(ExplodeParticles)
    {
    	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeParticles,
    		HitResult.Location, FRotator(0.f), true);
    }
	// 폭발후 피해 입히기
	

	// 폭발 후 해당 엑터 삭제
	Destroy();
}

