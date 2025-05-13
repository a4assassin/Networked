// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkedGameMode.h"
#include "NetworkedCharacter.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "UObject/ConstructorHelpers.h"

ANetworkedGameMode::ANetworkedGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ANetworkedGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (GameState)
	{
		int NumberOfPlayersJoined = GameState.Get()->PlayerArray.Num();
		GEngine->AddOnScreenDebugMessage(1, 40.f, FColor::Cyan, FString::Printf(TEXT("Number of players in session: %d"), NumberOfPlayersJoined));
	}

	APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
	if (PlayerState)
	{
		FString PlayerName = PlayerState->GetPlayerName();
		GEngine->AddOnScreenDebugMessage(-1, 40.f, FColor::Green, FString::Printf(TEXT("%s joined this session. "), *PlayerName));

	}
}

void ANetworkedGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (GameState)
	{
		int NumberOfPlayersJoined = GameState.Get()->PlayerArray.Num();
		GEngine->AddOnScreenDebugMessage(1, 40.f, FColor::Cyan, FString::Printf(TEXT("Number of players in session: %d"), NumberOfPlayersJoined));
	}

	APlayerState* PlayerState = Exiting->GetPlayerState<APlayerState>();
	if (PlayerState)
	{
		FString PlayerName = PlayerState->GetPlayerName();
		GEngine->AddOnScreenDebugMessage(-1, 40.f, FColor::Red, FString::Printf(TEXT("%s left this session. "), *PlayerName));

	}
}
