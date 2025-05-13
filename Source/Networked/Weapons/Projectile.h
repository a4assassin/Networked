// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class NETWORKED_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void Destroyed() override;
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* BoxCollisionComponent;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovmentComp;

private:
	UPROPERTY(EditAnywhere)
	class UParticleSystem* ProjectileTrace;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ProjectileImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	class UParticleSystemComponent* ProjectileTraceComponent;
};
