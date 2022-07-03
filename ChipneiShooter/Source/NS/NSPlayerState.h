// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NSGameMode.h"
#include "NSPlayerState.generated.h"

/**
 *
 */
UCLASS()
class NS_API ANSPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(Replicated)
	float m_fHealth = 100.f;
	
	UPROPERTY(Replicated)
	uint8 m_uiDeaths = 0u;

	UPROPERTY(Replicated)
	ETeam m_eTeam = ETeam::Team_BLUE;
	
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
