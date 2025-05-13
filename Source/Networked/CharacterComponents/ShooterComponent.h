// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShooterComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NETWORKED_API UShooterComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UShooterComponent();
	friend class AShooterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

private:

	class AShooterCharacter* ShooterCharacter;
	UPROPERTY(ReplicatedUsing = OnRep_WeaponEquipped)
	class AWeapon* EquippedWeapon;

	friend class AShooterCharacter;
	void Equip(AWeapon* Weapon);
	void SetAiming(bool bInIsAiming);

	UPROPERTY(Replicated)
	bool isAiming;

	UFUNCTION(Server, Reliable)
	void Server_SetAiming(bool bInIsAiming);

	UFUNCTION()
	void OnRep_WeaponEquipped();

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bIsFireButtonPressed;
	void FirePressed(bool bIsPressed);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& HitLocation);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& HitLocation);

	void TraceHit(FHitResult& HitResult);
	
	FVector HitPoint;
};
