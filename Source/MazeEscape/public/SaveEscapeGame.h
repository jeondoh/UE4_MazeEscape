// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveEscapeGame.generated.h"

// 저장 데이터
USTRUCT(BlueprintType)
struct FMazePlayerData
{
	GENERATED_BODY()

	// 캐릭터 체력
	UPROPERTY(VisibleAnywhere, Category="SaveGame|Data")
	float Health;
	// 캐릭터 최대체력
	UPROPERTY(VisibleAnywhere, Category="SaveGame|Data")
	float MaxHealth;
	// 장착한 장비명
	UPROPERTY(VisibleAnywhere, Category="SaveGame|Data")
	FString WeaponName;
	// 인벤토리 정보
	UPROPERTY(VisibleAnywhere, Category="SaveGame|Data")
	TArray<class AItem*> Inventory;
	// 레벨명
	UPROPERTY(VisibleAnywhere, Category="SaveGame|Data")
	FString LevelName;
	// 클리어 레벨명
	UPROPERTY(VisibleAnywhere, Category="SaveGame|Data")
	FString ClearGameLevel;
};


/**
 * 
 */
UCLASS()
class MAZEESCAPE_API USaveEscapeGame : public USaveGame
{
	GENERATED_BODY()

public:
	USaveEscapeGame();
	
	UPROPERTY(VisibleAnywhere, Category="SaveGame")
	FString PlayerName;
	
	UPROPERTY(VisibleAnywhere, Category="SaveGame")
	uint32 UserIndex;	

	UPROPERTY(VisibleAnywhere, Category="SaveGame")
	FMazePlayerData MazePlayerData;

	UPROPERTY(VisibleAnywhere, Category="SaveGame")
	FName ClearGameLevel;
};
