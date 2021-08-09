// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomCollision.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
ACustomCollision::ACustomCollision()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	// ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystem"));
	// ParticleSystemComponent->SetupAttachment(GetRootComponent());

	// 초기화
	IsCameraShake = false;
	IsPlaySound = false;
	IsParticle = false;
	ContainsText = TEXT("MazeCollision");
}

// Called when the game starts or when spawned
void ACustomCollision::BeginPlay()
{
	Super::BeginPlay();
	// 바인딩
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ACustomCollision::OnBoxOverlap);

}

void ACustomCollision::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor == nullptr) return;
	// 엑터명에 ContainsText에 포함되어 있으면
	bool IsContinue = OtherActor->GetName().Contains(ContainsText);
	if(IsContinue)
	{
		// 사운드 재생
		if(IsPlaySound && PlaySound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, PlaySound, GetActorLocation());
		}
		// 파티클 생성
		if(IsParticle && ParticleSystem)
		{
			UGameplayStatics::SpawnEmitterAtLocation(this, ParticleSystem, GetActorLocation());
		}
	}
}
