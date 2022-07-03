// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "NSGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "NSCharacter.h"
#include "NSHUD.h"
#include "NSPlayerState.h"
#include "NSSpawnPoint.h"

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

void ANSGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		//Create lists of SpawnPoints
		for (TActorIterator<ANSSpawnPoint> it(GetWorld()); it; ++it)
		{
			if ((*it)->m_eTeam == ETeam::Team_BLUE)
			{
				m_tSpawnPoint_Blue.Add(*it);
			}
			else if ((*it)->m_eTeam == ETeam::Team_RED)
			{
				m_tSpawnPoint_Red.Add(*it);
			}
		}
	}

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
