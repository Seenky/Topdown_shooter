// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Player\PlayerCharacter.h"
#include "ProjectileBase.generated.h"

UCLASS()
class SHOOT_N_RUN_API AProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectileBase();

	void FireInDirection(const FVector& ShootDirection);

protected:

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = Movement)
	UProjectileMovementComponent* ProjectileMovementComponent;

	void HandleFireInDirection(const FVector& ShootDirection);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireInDirection(const FVector& ShootDirection);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDestroyPlayer (APlayerCharacter* Player);

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
