// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MazeEscapeGameMode.generated.h"

UCLASS(minimalapi)
class AMazeEscapeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMazeEscapeGameMode();
	
protected:
    virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widgets", meta=(AllowPrivateAccess=true))
	TSubclassOf<class UUserWidget> HUDOverlayClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Widgets", meta=(AllowPrivateAccess=true))
	UUserWidget* HUDOverlay;
};



