// Fill out your copyright notice in the Description page of Project Settings.


#include "ShellCasing.h"
#include "Components/StaticMeshComponent.h"
AShellCasing::AShellCasing()
{
	PrimaryActorTick.bCanEverTick = false;

	ShellCasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShellCasing"));
	SetRootComponent(ShellCasingMesh);

	ShellCasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	ShellCasingMesh->SetSimulatePhysics(true);
	ShellCasingMesh->SetEnableGravity(true);
	ShellCasingMesh->SetNotifyRigidBodyCollision(true);

}

void AShellCasing::BeginPlay()
{
	Super::BeginPlay();

	ShellCasingMesh->AddImpulse(GetActorForwardVector() * 10.f);

	ShellCasingMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
}

void AShellCasing::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Destroy();
}

void AShellCasing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

