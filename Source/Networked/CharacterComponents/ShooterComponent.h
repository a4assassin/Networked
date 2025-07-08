// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Networked/HUD/ShooterHUD.h"
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
	void SetHUDCrosshairs(float DeltaTime);
private:

	class AShooterCharacter* ShooterCharacter;
	class AShooterHUD* ShooterHUD;
	class AShooterPlayerController* ShooterPlayerController;

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
	void InterpFOV(float DeltaTime);
	
	FVector HitPoint;
	FHUDPack HUDPack;

	float CrosshairVelocityFactor = 0.f;
	float CrosshairJumpFactor = 0.f;
	float CrosshairAimingFactor = 0.f;
	float CrosshairShootingFactor = 0.f;

	float DefaultFOV;
	float CurrentFOV;

	UPROPERTY(EditAnyWhere, Category = Combat)
	float ZoomFOV = 35.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;
};
