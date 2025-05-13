// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKED_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileItem;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AShellCasing> ShellCasingItem;

public:
	virtual void Fire(const FVector& HitTarget) override;
};
