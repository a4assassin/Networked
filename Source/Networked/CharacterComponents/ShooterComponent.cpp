// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterComponent.h"
#include "Networked/Characters/ShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Networked/Weapons/Weapon.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Networked/Characters/ShooterCharacter.h"

UShooterComponent::UShooterComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 0.f;
}


void UShooterComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UShooterComponent::Equip(AWeapon* Weapon)
{
	if (ShooterCharacter && Weapon)
	{
		EquippedWeapon = Weapon;
		const USkeletalMeshSocket* SocketName =  ShooterCharacter->GetMesh()->GetSocketByName(TEXT("RightHandSocket"));
		if (SocketName)
		{
			SocketName->AttachActor(EquippedWeapon, ShooterCharacter->GetMesh());
		}
		EquippedWeapon->SetOwner(ShooterCharacter);
		EquippedWeapon->ShowWeaponText(false);
		ShooterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		ShooterCharacter->bUseControllerRotationYaw = true;
	}
}

void UShooterComponent::SetAiming(bool bInIsAiming)
{
	isAiming = bInIsAiming;
	Server_SetAiming(bInIsAiming);
}

void UShooterComponent::OnRep_WeaponEquipped()
{
	if (EquippedWeapon && ShooterCharacter)
	{
		ShooterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		ShooterCharacter->bUseControllerRotationYaw = true;
	}
}

void UShooterComponent::FirePressed(bool bIsPressed)
{
	bIsFireButtonPressed = bIsPressed;
	if (bIsFireButtonPressed)
	{
		FHitResult HitResult;
		TraceHit(HitResult);
		ServerFire(HitResult.ImpactPoint);
	}
}

void UShooterComponent::TraceHit(FHitResult& HitResult)
{
	FVector2D ViewPortSize;
	if(GEngine && GEngine->GameViewport)
		GEngine->GameViewport->GetViewportSize(ViewPortSize);

	FVector2D CrossHairLocation(ViewPortSize / 2);
	FVector WorldPosition;
	FVector WorldDirection;
	bool isDeprojectSuccess = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrossHairLocation, WorldPosition, WorldDirection);

	if (isDeprojectSuccess)
	{
		FVector Start = WorldPosition;
		FVector End = Start + WorldDirection * 10000.f;
		GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (!HitResult.bBlockingHit)
		{
			HitResult.ImpactPoint = End;
			//HitPoint = End;
		}
		else
		{
			//HitPoint = HitResult.ImpactPoint;
			//DrawDebugBox(GetWorld(), HitPoint, FVector(20.f), FColor::Green);
		}
	}
}

void UShooterComponent::ServerFire_Implementation(const FVector_NetQuantize& HitLocation)
{
	MulticastFire(HitLocation);
}

void UShooterComponent::MulticastFire_Implementation(const FVector_NetQuantize& HitLocation)
{
	if (ShooterCharacter && EquippedWeapon)
	{
		ShooterCharacter->PlayFire(isAiming);
		EquippedWeapon->Fire(HitLocation);
	}
}
void UShooterComponent::Server_SetAiming_Implementation(bool bInIsAiming)
{
	isAiming = bInIsAiming;
	if (ShooterCharacter)
		ShooterCharacter->GetCharacterMovement()->MaxWalkSpeed = isAiming ? AimWalkSpeed : BaseWalkSpeed;
}



void UShooterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UShooterComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UShooterComponent, EquippedWeapon);
	DOREPLIFETIME(UShooterComponent, isAiming);
}

