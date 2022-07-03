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

void ANSGameMode::Respawn(ANSCharacter* _pChar)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		AController* pController = _pChar->GetController();
		_pChar->DetachFromControllerPendingDestroy();
		FTransform oTrans = _pChar->GetTransform();
		ANSCharacter* pNewChar = Cast<ANSCharacter>(GetWorld()->SpawnActor(DefaultPawnClass, &oTrans));
		if (pNewChar)
		{
			pController->Possess(pNewChar);
		}
	}
}
