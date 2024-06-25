// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;

    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;

}

void AWeaponBase::ShootBullet(const FRotator rot)
{
    FVector MuzzleLocation = WeaponMesh->GetSocketLocation(TEXT("MuzzleSocket"));
    UWorld* World = GetWorld();
    if (World)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();
        AProjectileBase* Projectile = World->SpawnActor<AProjectileBase>(ProjectileClass, MuzzleLocation, rot);
        if (Projectile)
        {
            // Set the projectile's initial trajectory.				
            FVector LaunchDirection = rot.Vector();
            Projectile->FireInDirection(LaunchDirection);
        }
    }
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

