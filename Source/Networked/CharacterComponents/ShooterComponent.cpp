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
#include "Networked/Characters/ShooterPlayerController.h"
#include "Networked/HUD/ShooterHUD.h"
#include "Camera/CameraComponent.h"

UShooterComponent::UShooterComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 0.f;
}


void UShooterComponent::BeginPlay()
{
	Super::BeginPlay();
	if (ShooterCharacter)
	{
		ShooterCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if(ShooterCharacter->GetFollowCamera())
		{
			DefaultFOV = ShooterCharacter->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
			
	}
	
}

void UShooterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	

	if (ShooterCharacter && ShooterCharacter->IsLocallyControlled())
	{
			FHitResult HitResult;
			TraceHit(HitResult);
			HitPoint = HitResult.ImpactPoint;
			SetHUDCrosshairs(DeltaTime);
			InterpFOV(DeltaTime);
	}
}

void UShooterComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (ShooterCharacter == nullptr || ShooterCharacter->Controller == nullptr) return;

	ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(ShooterCharacter->Controller) : ShooterPlayerController;
	if (ShooterPlayerController)
	{
		ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(ShooterPlayerController->GetHUD()) : ShooterHUD;
		if (ShooterHUD)
		{
			FHUDPack HUDPack;
			if (EquippedWeapon)
			{
				HUDPack.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPack.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPack.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPack.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPack.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			}
			else
			{
				HUDPack.CrosshairsCenter = nullptr;
				HUDPack.CrosshairsLeft = nullptr;
				HUDPack.CrosshairsRight = nullptr;
				HUDPack.CrosshairsBottom = nullptr;
				HUDPack.CrosshairsTop = nullptr;
			}

			FVector2D WalkSpeedRange(0.f, ShooterCharacter->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = FVector(ShooterCharacter->GetVelocity().X, ShooterCharacter->GetVelocity().Y, 0.f);

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if (ShooterCharacter->GetCharacterMovement()->IsFalling())
			{
				CrosshairJumpFactor = FMath::FInterpTo(CrosshairJumpFactor, 2.f, DeltaTime, 8.f);
			}
			else
			{
				CrosshairJumpFactor = FMath::FInterpTo(CrosshairJumpFactor, 0.f, DeltaTime, 20.f);
			}

			HUDPack.CalcCrosshairSpread = CrosshairVelocityFactor + CrosshairJumpFactor;
			ShooterHUD->SetHUDPack(HUDPack);
		}
	}

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

void UShooterComponent::InterpFOV(float DeltaTime)
{

	if (EquippedWeapon)
	{
		if (isAiming)
		{
			CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->ZoomFOV, DeltaTime, EquippedWeapon->ZoomInterpSpeed);
		}
		else
		{
			CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
		}

		if (ShooterCharacter && ShooterCharacter->GetFollowCamera())
		{
			ShooterCharacter->GetFollowCamera()->SetFieldOfView(CurrentFOV);
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





void UShooterComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UShooterComponent, EquippedWeapon);
	DOREPLIFETIME(UShooterComponent, isAiming);
}

