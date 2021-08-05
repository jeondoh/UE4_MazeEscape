// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "BulletHitInterface.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

UCLASS()
class MAZEESCAPE_API AEnemy : public ACharacter, public IBulletHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// BulletHitInterface에서 Override
	virtual void BulletHit_Implementation(FHitResult HitResult) override;
	
	// 데미지 피해량
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

private:
	// 총에 맞았을때 파티클
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Particles", meta=(AllowPrivateAccess=true))
	class UParticleSystem* ImpactParticles;
	// 총에 맞았을때 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|Sounds", meta=(AllowPrivateAccess=true))
	class USoundCue* ImpactSound;
	// 현재체력
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	float Health;
	// 최대체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy|State", meta=(AllowPrivateAccess=true))
	float MaxHealth;
	
};
