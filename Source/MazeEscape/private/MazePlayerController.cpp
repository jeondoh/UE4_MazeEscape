// Fill out your copyright notice in the Description page of Project Settings.


#include "MazePlayerController.h"

#include "Blueprint/UserWidget.h"

AMazePlayerController::AMazePlayerController()
{
	
}

void AMazePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if(HUDOverlayClass)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayClass);
		if(HUDOverlay)
		{
			HUDOverlay->AddToViewport();
			HUDOverlay->SetVisibility(ESlateVisibility::Visible);
		}
	}
}
