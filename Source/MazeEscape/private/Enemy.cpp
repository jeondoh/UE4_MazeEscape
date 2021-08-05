// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	// 초기화
	InitalizedData();
	
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdateHitNumbers();
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::InitalizedData()
{
	Health = 100.f;
	MaxHealth = 100.f;
	HealthBarDisplayTime = 4.f;
	HitReactTimeMin = 0.5f;
	HitReactTimeMax = 0.75f;
	bCanHitReact = true;
	HitNumberDestroyTime = 1.5f;
}

void AEnemy::BulletHit_Implementation(FHitResult HitResult)
{
	// 사운드
	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	// 파티클
	if(ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles,
			HitResult.Location, FRotator(0.f), true);
	}
	// 히트/사망 애니메이션 몽타주
	PlayHitMontage(FName("HitReactFront"), 1.0f);
	// 적 체력바 보이기(쏘고나서 이후 HealthBarDisplayTime 동안만) 
	ShowHealthBar();
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	Health -= DamageAmount;

	if(Health <= 0.f)
	{
		Health = 0.f;
		Die();
	}
	return DamageAmount;
}

void AEnemy::ShowHealthBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(HealthBarTimer);
	GetWorldTimerManager().SetTimer(HealthBarTimer, this, &AEnemy::HideHealthBar, HealthBarDisplayTime);
}

void AEnemy::Die()
{
	// 체력바 숨기기
	HideHealthBar();
}

void AEnemy::PlayHitMontage(FName Section, float PlayRate)
{
	if(bCanHitReact)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if(AnimInstance)
		{
			AnimInstance->Montage_Play(HitMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(Section, HitMontage);
		}
		// 빠르게 총을 쏠때 히트 애니메이션 재생 간격을 주기 위해 
		bCanHitReact = false;
		const float HitReactTime{FMath::FRandRange(HitReactTimeMin, HitReactTimeMax)};
		GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReactTimer, HitReactTime);
	}
}

void AEnemy::ResetHitReactTimer()
{
	bCanHitReact = true;
}

void AEnemy::StoreHitNumber(UUserWidget* HitNumber, FVector Location)
{
	HitNumbers.Add(HitNumber, Location);

	FTimerHandle HitNumberTimer;
	FTimerDelegate HitNumberDelegate;
	HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumber); // 입력 매개변수를 사용해 바인딩 가능
	GetWorld()->GetTimerManager().SetTimer(HitNumberTimer, HitNumberDelegate, HitNumberDestroyTime, false);
}

void AEnemy::DestroyHitNumber(UUserWidget* HitNumber)
{
	HitNumbers.Remove(HitNumber);
	HitNumber->RemoveFromParent();
}

void AEnemy::UpdateHitNumbers()
{
	for(auto& HitPair : HitNumbers)
	{
		UUserWidget* HitNumber{HitPair.Key};
		const FVector Location{HitPair.Value};
		FVector ResultLocation = FVector(Location.X, Location.Y, Location.Z + 30.f);
		FVector2D ScreenPosition;

		UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), ResultLocation, ScreenPosition);
		HitNumber->SetPositionInViewport(ScreenPosition);
	}	
}
