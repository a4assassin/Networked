// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Networked/Weapons/Weapon.h"


void UShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	
}

void UShooterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (ShooterCharacter == nullptr) return;

	FVector Velocity = ShooterCharacter->GetMovementComponent()->Velocity;
	Velocity.Z = 0.f;
	Speed = Velocity.Length();

	isInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();
	
	isAccelerating = ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Length() > 0.f;// ? true : false;

	isWeaponEquipped = ShooterCharacter->IsWeaponEquipped();

	isCrouched = ShooterCharacter->bIsCrouched;
	isAiming = ShooterCharacter->IsAiming();

	TurninPlace = ShooterCharacter->GetTurnInPlace();

	FRotator Aim = ShooterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());

	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, Aim);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 5.f);
	YawRotationOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterCurrentRotation;
	CharacterCurrentRotation = ShooterCharacter->GetActorRotation();
	FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterCurrentRotation, CharacterRotationLastFrame);
	const float TargetYaw = Delta.Yaw / DeltaTime;
	const float InterpolateRotation = FMath::FInterpTo(Leaning, TargetYaw, DeltaTime, 5.f);

	Leaning = FMath::Clamp(InterpolateRotation, -100.f, 100.f);

	AimOffsetYaw = ShooterCharacter->GetAO_Yaw();
	AimOffsetPitch = ShooterCharacter->GetAO_Pitch();

	if (isWeaponEquipped && ShooterCharacter->GetEquippedWeapon() && ShooterCharacter->GetMesh())
	{
		LeftHandTransformOnWeapon = ShooterCharacter->GetEquippedWeapon()->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutLocation;
		FRotator OutRotator;
		ShooterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransformOnWeapon.GetLocation(), FRotator::ZeroRotator, OutLocation, OutRotator);
		LeftHandTransformOnWeapon.SetLocation(OutLocation);
		LeftHandTransformOnWeapon.SetRotation(FQuat(OutRotator));
	
	}
}