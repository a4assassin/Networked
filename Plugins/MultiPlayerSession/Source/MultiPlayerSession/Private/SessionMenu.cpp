// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionMenu.h"
#include "Components/Button.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "MultiplayerSubsystem.h"


void USessionMenu::ConstructWidget(int AllowedConnections, FString TypeOfSession, FString InLobbyPath)
{
	LobbyPath = FString::Printf(TEXT("%s?listen"), *InLobbyPath);
	NumberOfAllowedConnections = AllowedConnections;
	MatchType = TypeOfSession;

	AddToViewport();
	SetIsFocusable(true);

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSubsystem = GameInstance->GetSubsystem<UMultiplayerSubsystem>();
		if (MultiplayerSubsystem)
		{
			MultiplayerSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
			MultiplayerSubsystem->MultiplayerOnFindSessionComplete.AddUObject(this, &ThisClass::OnFindSession);
			MultiplayerSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
			MultiplayerSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
			MultiplayerSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
		}
	}
}

bool USessionMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (HostButton && JoinButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}
	return true;
}

void USessionMenu::NativeDestruct()
{
	EscapeUI();
	Super::NativeDestruct();
}

void USessionMenu::OnCreateSession(bool isSuccessful)
{
	if (isSuccessful)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Session Created. Travelling now."));
			World->ServerTravel(*LobbyPath);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Session Creation Failed from Menu class"));
		HostButton->SetIsEnabled(true);
	}

}

void USessionMenu::OnFindSession(const TArray<FOnlineSessionSearchResult>& Results, bool isSuccessful)
{
	if (MultiplayerSubsystem != nullptr)
	{
		FString ValueInSettings;
		for (auto Result : Results)
		{
			Result.Session.SessionSettings.Get(FName("MatchType"), ValueInSettings);
			if (ValueInSettings == MatchType)
			{
				MultiplayerSubsystem->JoinSession(Result);
				return;
			}
		}
		if (isSuccessful || Results.Num() == 0)
		{
			JoinButton->SetIsEnabled(true);
		}
	}
}

void USessionMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}


void USessionMenu::OnDestroySession(bool isSuccessful)
{
}

void USessionMenu::OnStartSession(bool isSuccessful)
{
}

void USessionMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Host clicked"));
	if (MultiplayerSubsystem)
	{
		MultiplayerSubsystem->CreateSession(NumberOfAllowedConnections, MatchType);
	}
}

void USessionMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Join clicked"));
	if (MultiplayerSubsystem)
	{
		MultiplayerSubsystem->FindSession(1000);
	}
}
