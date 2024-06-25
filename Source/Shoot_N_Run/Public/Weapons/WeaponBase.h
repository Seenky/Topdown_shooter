// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapons/Projectiles/ProjectileBase.h"
#include "WeaponBase.generated.h"

UCLASS()
class SHOOT_N_RUN_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

	void ShootBullet(const FRotator rot);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* WeaponMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<class AProjectileBase> ProjectileClass;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
