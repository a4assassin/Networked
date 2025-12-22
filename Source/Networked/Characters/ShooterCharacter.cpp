// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Networked/Weapons/Weapon.h"
#include "Networked/CharacterComponents/ShooterComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Networked/Networked.h"


AShooterCharacter::AShooterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->bUsePawnControlRotation = true;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	ShooterComponent = CreateDefaultSubobject <UShooterComponent> (TEXT("Weapon"));
	ShooterComponent->SetIsReplicated(true);

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	TurnInPlace = ETurnType::ETT_NoTurn;
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetAimOffsets(DeltaTime);
	HideCameraIfCharacterClose();
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("Forward", this, &ThisClass::Moveforward);
	PlayerInputComponent->BindAxis("Right", this, &ThisClass::MoveRight);
	PlayerInputComponent->BindAxis("LookRight", this, &ThisClass::LookRight);
	PlayerInputComponent->BindAxis("LookUp", this, &ThisClass::LookUp);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ThisClass::Jump);
	PlayerInputComponent->BindAction("Equip", EInputEvent::IE_Pressed, this, &ThisClass::Equip);
	PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &ThisClass::CrouchPressed);
	PlayerInputComponent->BindAction("Aim", EInputEvent::IE_Pressed, this, &ThisClass::AimPressed);
	PlayerInputComponent->BindAction("Aim", EInputEvent::IE_Released, this, &ThisClass::AimReleased);
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &ThisClass::FirePressed);
	PlayerInputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &ThisClass::FireReleased);
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AShooterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void AShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (ShooterComponent)
	{
		ShooterComponent->ShooterCharacter = this;
	}
}

void AShooterCharacter::SetAimOffsets(float DeltaTime)
{
	if (ShooterComponent && ShooterComponent->EquippedWeapon)
	{
		FVector Velocity = GetVelocity();
		float Speed = FVector(Velocity.X, Velocity.Y, 0.f).Length();
		bool isInAir = GetCharacterMovement()->IsFalling();

		if (Speed == 0.f && !isInAir)
		{
			bUseControllerRotationYaw = true;
			FRotator CurrentRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);;
			FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentRotation, StartingRotation);
			AO_Yaw = DeltaRotation.Yaw;
			if (TurnInPlace == ETurnType::ETT_NoTurn)
			{
				Interpolating_AO_Yaw = AO_Yaw;
			}
			ApplyTurnInPlace(DeltaTime);
		}

		if (Speed > 0.f || isInAir)
		{
			bUseControllerRotationYaw = true;
			StartingRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
			AO_Yaw = 0.f;
			TurnInPlace = ETurnType::ETT_NoTurn;
		}
		AO_Pitch = GetBaseAimRotation().Pitch;
		if (AO_Pitch > 90.f && !IsLocallyControlled())
		{
			FVector2D InRange(270.f, 360.f);
			FVector2D OutRange(-90.f, 0.f);
			AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
		}
	}
}

void AShooterCharacter::ApplyTurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurnInPlace = ETurnType::ETT_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurnInPlace = ETurnType::ETT_Left;
	}
	if (TurnInPlace != ETurnType::ETT_NoTurn)
	{
		Interpolating_AO_Yaw = FMath::FInterpTo(Interpolating_AO_Yaw, 0.f, DeltaTime, 10.f);
		AO_Yaw = Interpolating_AO_Yaw;
		if (FMath::Abs(AO_Yaw) < 10.f)
		{
			TurnInPlace = ETurnType::ETT_NoTurn;
			StartingRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void AShooterCharacter::HideCameraIfCharacterClose()
{
	if (IsLocallyControlled())
	{
		if ((Camera->GetComponentLocation() - GetActorLocation()).Size() < CameraDistanceThreshold)
		{
			GetMesh()->SetVisibility(false);
			if (ShooterComponent && ShooterComponent->EquippedWeapon && ShooterComponent->EquippedWeapon->GetWeaponMesh())
			{
				ShooterComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
			}
		}
		else
		{
			GetMesh()->SetVisibility(true);
			if (ShooterComponent && ShooterComponent->EquippedWeapon && ShooterComponent->EquippedWeapon->GetWeaponMesh())
			{
				ShooterComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
			}
		}
	}
}

void AShooterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		OverlappingWeapon->ShowWeaponText(true);
	}
}

bool AShooterCharacter::IsWeaponEquipped()
{
	return (ShooterComponent && ShooterComponent->EquippedWeapon);
}

bool AShooterCharacter::IsAiming()
{
	return (ShooterComponent && ShooterComponent->isAiming);
}

AWeapon* AShooterCharacter::GetEquippedWeapon() const
{
	if (!ShooterComponent->EquippedWeapon) return nullptr;
	return ShooterComponent->EquippedWeapon;
}

void AShooterCharacter::PlayFireAnimMontage(bool isAiming)
{
	if (ShooterComponent && ShooterComponent->EquippedWeapon)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && FireAnimMontage)
		{
			AnimInstance->Montage_Play(FireAnimMontage);
			FName Section;
			Section = isAiming ? FName("Aim") : FName("Hip");
			AnimInstance->Montage_JumpToSection(Section);
		}
	}
}

void AShooterCharacter::MulticastHitReact_Implementation()
{
	PlayHitReactAnimMontage(FName("FromFront"));
}

void AShooterCharacter::PlayHitReactAnimMontage(FName HitAnimName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactAnimMontage)
	{
		AnimInstance->Montage_Play(HitReactAnimMontage);
		AnimInstance->Montage_JumpToSection(HitAnimName);
	}
}

FVector AShooterCharacter::GetHitTargetPoint() const
{
	if (ShooterComponent == nullptr)
		return FVector();
	return ShooterComponent->HitPoint;
}

void AShooterCharacter::Moveforward(float Scale)
{
	if (Controller != nullptr && Scale != 0.f)
	{
		FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, Scale);
	}
}

void AShooterCharacter::MoveRight(float Scale)
{
	if (Controller != nullptr && Scale != 0.f)
	{
		FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, Scale);
	}
}

void AShooterCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}

}

void AShooterCharacter::LookUp(float Scale)
{
	AddControllerPitchInput(Scale);
}

void AShooterCharacter::LookRight(float Scale)
{
	//GEngine->AddOnScreenDebugMessage(0, 1.f, FColor::Red, FString::Printf(TEXT("%d"), &Scale));
	AddControllerYawInput(Scale);
}

void AShooterCharacter::Equip()
{
	FString BoolAsString = (ShooterComponent && OverlappingWeapon && HasAuthority()) ? TEXT("True") : TEXT("False");
	GEngine->AddOnScreenDebugMessage(1, 3, FColor::Red, BoolAsString);
	if (ShooterComponent && OverlappingWeapon)
	{
		if (HasAuthority())
		{
			ShooterComponent->Equip(OverlappingWeapon);
		}
		else
		{
			ServerEquip();
		}
	}
}

void AShooterCharacter::CrouchPressed()
{
	if (bIsCrouched)
		UnCrouch();
	else
		Crouch();
}

void AShooterCharacter::AimPressed()
{
	ShooterComponent->SetAiming(true);
}

void AShooterCharacter::AimReleased()
{
	ShooterComponent->SetAiming(false);
}

void AShooterCharacter::FirePressed()
{
	if (ShooterComponent)
	{
		ShooterComponent->FirePressed(true);
	}
}

void AShooterCharacter::FireReleased()
{
	if (ShooterComponent)
	{
		ShooterComponent->FirePressed(false);
	}
}


void AShooterCharacter::ServerEquip_Implementation()
{
	if (ShooterComponent)
	{
		ShooterComponent->Equip(OverlappingWeapon);
	}
}

void AShooterCharacter::OnRep_OverlappingWeapon()
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowWeaponText(true);
	}

}





