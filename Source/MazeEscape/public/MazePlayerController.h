// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MazePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MAZEESCAPE_API AMazePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMazePlayerController();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widgets", meta=(AllowPrivateAccess=true))
	TSubclassOf<class UUserWidget> HUDOverlayClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Widgets", meta=(AllowPrivateAccess=true))
	UUserWidget* HUDOverlay;
};
