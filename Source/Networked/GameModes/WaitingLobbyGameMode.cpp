// Fill out your copyright notice in the Description page of Project Settings.


#include "WaitingLobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void AWaitingLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (GameState.Get()->PlayerArray.Num() == 2)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString("/Game/Maps/Village?listen"));
			
		}
	}
}