// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"
#include "ShellCasing.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	if (HasAuthority())
	{
		const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
		if (MuzzleSocket)
		{
			UE_LOG(LogTemp, Warning, TEXT("Spawning"));
			FTransform MuzzleSocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());

			FVector HitPoint = HitTarget - MuzzleSocketTransform.GetLocation();
			FRotator HitPointRotation = HitPoint.Rotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = Cast<APawn>(GetOwner());

			GetWorld()->SpawnActor<AProjectile>(ProjectileItem, MuzzleSocketTransform.GetLocation(), HitPointRotation, SpawnParams);
		}
	}
	
	if (ShellCasingItem)
	{
		const USkeletalMeshSocket* EjectSocket =  GetWeaponMesh()->GetSocketByName(FName("AmmoEject"));
		
		if (EjectSocket)
		{
			FTransform ShellTransform = EjectSocket->GetSocketTransform(GetWeaponMesh());
			GetWorld()->SpawnActor<AShellCasing>(ShellCasingItem, ShellTransform);
		}
	}
}
