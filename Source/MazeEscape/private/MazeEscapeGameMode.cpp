// Copyright Epic Games, Inc. All Rights Reserved.

#include "MazeEscapeGameMode.h"
#include "MazeEscapeHUD.h"
#include "MazeEscapeCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMazeEscapeGameMode::AMazeEscapeGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AMazeEscapeHUD::StaticClass();
}
