// Copyright Epic Games, Inc. All Rights Reserved.

#include "MazeEscapeGameMode.h"
#include "MazeEscapeHUD.h"
#include "Blueprint/UserWidget.h"


AMazeEscapeGameMode::AMazeEscapeGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	// static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	// DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AMazeEscapeHUD::StaticClass();
}

void AMazeEscapeGameMode::BeginPlay()
{
	if(HUDOverlayClass)
	{
		HUDOverlay = CreateWidget<UUserWidget>(GetWorld(), HUDOverlayClass);
		if(HUDOverlay)
		{
			HUDOverlay->AddToViewport();
			HUDOverlay->SetVisibility(ESlateVisibility::Visible);
		}
	}

	if(GameOverUIFactory)
	{
		GameOverUI = CreateWidget<UUserWidget>(GetWorld(), GameOverUIFactory);
		if(GameOverUI)
		{
			GameOverUI->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}


	



