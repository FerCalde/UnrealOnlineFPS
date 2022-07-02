// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "NSGameMode.h"
#include "NSHUD.h"
#include "NSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "NSPlayerState.h"

ANSGameMode::ANSGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ANSHUD::StaticClass();
	// use our custom player state class
	PlayerStateClass = ANSPlayerState::StaticClass();
}
