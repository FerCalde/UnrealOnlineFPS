// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NSGameMode.generated.h"

class ANSCharacter;

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

	void Respawn(ANSCharacter* _pChar);
};



