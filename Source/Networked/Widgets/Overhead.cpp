// Fill out your copyright notice in the Description page of Project Settings.


#include "Overhead.h"
#include "Components/TextBlock.h"

void UOverhead::SetDisplayText(FString InText)
{
	DisplayText->SetText(FText::FromString(InText));
}

void UOverhead::ShowPlayerNetworkRole(APawn* InPawn)
{
	ENetRole RemoteRole =  InPawn->GetRemoteRole();
	FString Role;
	switch (RemoteRole)
	{
	case ROLE_None:
		Role = FString("None");
		break;
	case ROLE_SimulatedProxy:
		Role = FString("SimulatedProxy");
		break;
	case ROLE_AutonomousProxy:
		Role = FString("AutonomousProxy");
		break;
	case ROLE_Authority:
		Role = FString("Authority");
		break;
	}

	SetDisplayText(FString::Printf(TEXT("RemoteRole: %s"), *Role));
}
