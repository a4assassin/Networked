// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SessionMenu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSION_API USessionMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void ConstructWidget(int AllowedConnections, FString TypeOfSession, FString InLobbyPath);

	UFUNCTION(BlueprintImplementableEvent)
	void EscapeUI();

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnCreateSession(bool isSuccessful);
	void OnFindSession(const TArray<FOnlineSessionSearchResult>& Results, bool isSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool isSuccessful);
	UFUNCTION()
	void OnStartSession(bool isSuccessful);

	FString LobbyPath{ TEXT("") };

private:
	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();


	class UMultiplayerSubsystem* MultiplayerSubsystem;

	int NumberOfAllowedConnections{ 4 };
	FString MatchType{ TEXT("FreeForAll") };
	

	
};
