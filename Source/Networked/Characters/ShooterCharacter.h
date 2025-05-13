// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Networked/TurnType.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class NETWORKED_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()
private:

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon();

	FRotator StartingRotation = FRotator().ZeroRotator;

	float AO_Yaw;
	float Interpolating_AO_Yaw;
	float AO_Pitch;

	void SetAimOffsets(float DeltaTime);
	void ApplyTurnInPlace(float DeltaTime);
	ETurnType TurnInPlace;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	class UShooterComponent* ShooterComponent;

	void Moveforward(float Scale);
	void MoveRight(float Scale);
	virtual void Jump() override;
	void LookUp(float Scale);
	void LookRight(float Scale);
	void Equip();
	void CrouchPressed();
	void AimPressed();
	void AimReleased();
	void FirePressed();
	void FireReleased();

	UPROPERTY(EditAnywhere)
	class UAnimMontage* FireAnimMontage;

	UFUNCTION(Server, Reliable)
	void ServerEquip();

public:
	AShooterCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	void SetOverlappingWeapon(class AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();

	FORCEINLINE float GetAO_Yaw() const{ return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurnType GetTurnInPlace() const { return TurnInPlace; }
	 
	AWeapon* GetEquippedWeapon() const;

	void PlayFire(bool isAiming);
};
