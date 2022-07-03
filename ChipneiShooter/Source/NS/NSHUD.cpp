// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "NSHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"
#include "NSCharacter.h"
#include "NSPlayerState.h"
#include "Kismet/GameplayStatics.h"


ANSHUD::ANSHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/FirstPerson/Textures/FirstPersonCrosshair"));
	CrosshairTex = CrosshairTexObj.Object;
}


void ANSHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw very simple crosshair

	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition((Center.X),
		(Center.Y/* + 20.0f*/));

	// draw the crosshair
	FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TileItem);


	// draw the health of the player
	ANSCharacter* pChar = Cast<ANSCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	ANSPlayerState* pPlayerState = pChar ? Cast<ANSPlayerState>(pChar->GetPlayerState()) : nullptr; //EsLoMismo!//if(pChar){ANSPlayerState* pPlayerState = Cast<ANSPlayerState>(pChar->GetPlayerState());}

	if (pPlayerState)
	{
		FString sHealth = FString::Printf(TEXT("Health: %f"), pPlayerState->m_fHealth);
		DrawText(sHealth, FColor::Yellow, 50.0f, 50.0f);
		FString sDeaths = FString::Printf(TEXT("Death: %i"), pPlayerState->m_uiDeaths);
		DrawText(sDeaths, FColor::Red, 50.0f, 100.0f);
		FString sScore = FString::Printf(TEXT("Kills: %f"), pPlayerState->GetScore());
		DrawText(sScore, FColor::Green, 50.0f, 200.0f);
	}

}
