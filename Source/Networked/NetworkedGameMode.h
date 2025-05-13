// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NetworkedGameMode.generated.h"

UCLASS(minimalapi)
class ANetworkedGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANetworkedGameMode();

private:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
};



