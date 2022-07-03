// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NSGameMode.generated.h"

class ANSCharacter;
class ANSSpawnPoint;

UENUM(BlueprintType)
enum class EWaveState : uint8
{
	WaitingToStart,
	WaveInProgress,
	WaitingToComplete,
	GameOver
};

UENUM(BlueprintType)
enum class ETeam : uint8
{
	None,
	Team_RED,
	Team_BLUE
};


UCLASS(minimalapi)
class ANSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANSGameMode();


	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PostLogin(APlayerController* _pPC) override; //"BeginPlay" de na mas conectarse un jugador 

	void Spawn(ANSCharacter* _pCharacter);
	void Respawn(ANSCharacter* _pChar);


private:
	//Team Control
	TArray<ANSCharacter*> m_tTeam_Red;
	TArray<ANSCharacter*> m_tTeam_Blue;
	TArray<ANSSpawnPoint*> m_tSpawnPoint_Red;
	TArray<ANSSpawnPoint*> m_tSpawnPoint_Blue;
	TArray<ANSCharacter*> m_tToBeSpawned;
};



