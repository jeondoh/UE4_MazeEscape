// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "BulletHitInterface.h"
#include "GameFramework/Actor.h"
#include "Explosive.generated.h"

UCLASS()
class MAZEESCAPE_API AExplosive : public AActor, public IBulletHitInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExplosive();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/**************************************************************************************************/
	/** 인터페이스 Override **/
	virtual void BulletHit_Implementation(FHitResult HitResult, AActor* Player, AController* InstigatorController) override;

private:
	// 폭발 효과
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Explosive|Particles", meta=(AllowPrivateAccess=true))
	class UParticleSystem* ExplodeParticles;
	// 폭발 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Explosive|Sounds", meta=(AllowPrivateAccess=true))
	class USoundCue* ExplodeSound;
	// 스태틱 매쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Explosive|Component", meta=(AllowPrivateAccess=true))
	class UStaticMeshComponent* ExplosiveMesh;
	// 폭발 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Explosive|Component", meta=(AllowPrivateAccess=true))
	class USphereComponent* OverlapSphere;
	// 폭발 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Explosive|Damage", meta=(AllowPrivateAccess=true))
	float ExplosvieDamage;
};
