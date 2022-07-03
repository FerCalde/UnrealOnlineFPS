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

		//Spawn the players
		APlayerController* pPlayerController = GetWorld()->GetFirstPlayerController();
		if (pPlayerController)
		{

			ANSCharacter* pChar = pPlayerController->GetPawn<ANSCharacter>();
			if (pChar)
			{
				pChar->SetTeam(ETeam::Team_BLUE);
				m_tTeam_Blue.Add(pChar);
				Spawn(pChar);
			}
		}
	}

}

void ANSGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (GetLocalRole() == ROLE_Authority)
	{
		for (ANSCharacter* pChar : m_tToBeSpawned)
		{
			Spawn(pChar);
		}
	}
}

void ANSGameMode::PostLogin(APlayerController* _pPC)
{
	Super::PostLogin(_pPC);

	if (GetLocalRole() == ROLE_Authority)
	{
		ANSCharacter* pTeamLess = _pPC->GetPawn<ANSCharacter>();
		if (pTeamLess)
		{
			ANSPlayerState* pPS = _pPC->GetPlayerState<ANSPlayerState>();
			if (pPS)
			{
				if (m_tTeam_Blue.Num() > m_tTeam_Red.Num())
				{
					m_tTeam_Red.Add(pTeamLess);
					pTeamLess->SetTeam(ETeam::Team_RED);
				}
				else
				{
					m_tTeam_Blue.Add(pTeamLess);
					pTeamLess->SetTeam(ETeam::Team_BLUE);
				}

				pTeamLess->SetTeam(pPS->m_eTeam);
				Spawn(pTeamLess);


				for (ANSCharacter* pChar : m_tTeam_Blue)
				{
					ANSPlayerState* pAuxPS = pChar->GetPlayerState<ANSPlayerState>();
					if (pAuxPS)
					{
						pChar->SetTeam(pAuxPS->m_eTeam);
					}
				}
				for (ANSCharacter* pChar : m_tTeam_Red)
				{
					ANSPlayerState* pAuxPS = pChar->GetPlayerState<ANSPlayerState>();
					if (pAuxPS)
					{
						pChar->SetTeam(pAuxPS->m_eTeam);
					}
				}				
			}
		}
	}
}

void ANSGameMode::Spawn(ANSCharacter* _pCharacter)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		ANSSpawnPoint* pSpawnPoint = nullptr;
		TArray<ANSSpawnPoint*>* pTargetTeam;
		if (_pCharacter->GetPlayerState<ANSPlayerState>()->m_eTeam == ETeam::Team_BLUE)
		{
			pTargetTeam = &m_tSpawnPoint_Blue;
		}
		else if (_pCharacter->GetPlayerState<ANSPlayerState>()->m_eTeam == ETeam::Team_RED)
		{
			pTargetTeam = &m_tSpawnPoint_Red;
		}
		else
		{
			return;
		}


		for (ANSSpawnPoint* pAuxSpawnPoint : *pTargetTeam)
		{
			if (!pAuxSpawnPoint->GetBlocked())
			{
				if (m_tToBeSpawned.Contains(_pCharacter))
				{
					m_tToBeSpawned.Remove(_pCharacter);
				}
				_pCharacter->SetActorLocation(pAuxSpawnPoint->GetActorLocation());
				pAuxSpawnPoint->UpdateOverlaps(); //Vuelve a checkear los overlaps. Necesario para bugfix. Evita que se creen varios actores en el mismo SP por no haber hecho este checkeo entre ticks
				return;
				//break;
			}
		}
		if (m_tToBeSpawned.Contains(_pCharacter))
		{
			//When all spawn points are blocked. Add to the list of ToBeSpawn
			m_tToBeSpawned.Add(_pCharacter);
		}

		//Random patrol->TO CHECK! Copilot do it
		/*
		//Get a random spawn point
		ANSSpawnPoint* pSpawnPoint = nullptr;
		if (_pCharacter->m_eTeam == ETeam::Team_BLUE)
		{
			pSpawnPoint = m_tSpawnPoint_Blue[FMath::RandRange(0, m_tSpawnPoint_Blue.Num() - 1)];
		}
		else if (_pCharacter->m_eTeam == ETeam::Team_RED)
		{
			pSpawnPoint = m_tSpawnPoint_Red[FMath::RandRange(0, m_tSpawnPoint_Red.Num() - 1)];
		}
		else
		{
			return;
		}
		//Spawn the character
		_pCharacter->SpawnAt(pSpawnPoint->GetActorLocation(), pSpawnPoint->GetActorRotation());
		*/
	}
}

void ANSGameMode::Respawn(ANSCharacter* _pChar)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		AController* pController = _pChar->GetController();
		_pChar->DetachFromControllerPendingDestroy();
		//FTransform oTrans = _pChar->GetTransform();
		//ANSCharacter* pNewChar = Cast<ANSCharacter>(GetWorld()->SpawnActor(DefaultPawnClass, &oTrans));
		ANSCharacter* pNewChar = Cast<ANSCharacter>(GetWorld()->SpawnActor(DefaultPawnClass));
		if (pNewChar)
		{
			pController->Possess(pNewChar);
			Spawn(pNewChar);

			ANSPlayerState* pPS = pController->GetPlayerState<ANSPlayerState>();
			if (pPS)
			{
				pNewChar->SetTeam(pPS->m_eTeam);
			}
		}
	}
}
