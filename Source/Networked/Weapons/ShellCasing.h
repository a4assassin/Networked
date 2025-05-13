// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShellCasing.generated.h"

UCLASS()
class NETWORKED_API AShellCasing : public AActor
{
	GENERATED_BODY()
	
public:	
	AShellCasing();
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;

private:	
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* ShellCasingMesh;
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

};
