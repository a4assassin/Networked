// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Overhead.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKED_API UOverhead : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	void SetDisplayText(FString InText);

	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetworkRole(APawn* InPawn);

	//virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;
};
