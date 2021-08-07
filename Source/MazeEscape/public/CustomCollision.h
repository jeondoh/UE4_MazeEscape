// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CustomCollision.generated.h"

UCLASS()
class MAZEESCAPE_API ACustomCollision : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACustomCollision();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	// 박스 Collision
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Collision|Component", meta=(AllowPrivateAccess))
	class UBoxComponent* BoxComponent;
	
	// Collision 충돌시 카메라 흔들림 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision|Camera", meta=(AllowPrivateAccess))
	bool IsCameraShake;
	
	// Collision 충돌시 사운드 재생 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision|Sound", meta=(AllowPrivateAccess))
	bool IsPlaySound;
	
	// Collision 충돌시 사운드 재생
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision|Sound", meta=(AllowPrivateAccess))
	class USoundCue* PlaySound;

	// Collision 충돌시 파티클 효과 재생 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision|Particle", meta=(AllowPrivateAccess))
	bool IsParticle;

	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Collision|Particle", meta=(AllowPrivateAccess))
	// class UParticleSystemComponent* ParticleSystemComponent;
	
	// Collision 충돌시 파티클 효과 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision|Particle", meta=(AllowPrivateAccess))
	class UParticleSystem* ParticleSystem;

	// 부딪힐 엑터이름에 포함된 문자 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision|Text", meta=(AllowPrivateAccess))
	FString ContainsText;

	UFUNCTION(BlueprintCallable)
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
};
