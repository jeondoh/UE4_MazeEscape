// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelTransitionVolume.generated.h"

UCLASS()
class MAZEESCAPE_API ALevelTransitionVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALevelTransitionVolume();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	// 이동할 레벨명
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Transition", meta=(AllowPrivateAccess))
	FName TransitionLevelName;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Transition", meta=(AllowPrivateAccess))
	class UBoxComponent* TransitionVolume;

	UPROPERTY()
	class UBillboardComponent* BillboardComponent;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

};
