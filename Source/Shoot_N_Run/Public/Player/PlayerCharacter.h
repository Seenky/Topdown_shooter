// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Weapons\WeaponBase.h"
#include "PlayerCharacter.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config = Game)
class SHOOT_N_RUN_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    APlayerCharacter();   
    
protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FRotator WeaponRotation = FRotator(0, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FVector WeaponLocation = FVector(0, 0, 0);

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    TSubclassOf<class AWeaponBase> WeaponClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    AWeaponBase* CurrentWeapon;

    UPROPERTY(Replicated)
    FRotator rot;

    UPROPERTY(Replicated)
    FVector ShootDirection;

protected:

    FTimerHandle ShootTimerHandle;

    // Default Mapping Context
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputMappingContext* DefaultMappingContext;

    // Move Action
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* MoveAction;

    // Sprint Action
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* SprintAction;

    // Shoot Action
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* ShootAction;

    // Server function to handle sprinting
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSprint();

    // Server function to handle shooting
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerShoot(bool bShouldShoot);

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRotateToMouse(FRotator NewRotation);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastSetActorRotation(FRotator NewRotation);

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Function to move the character
    void Move(const FInputActionValue& Value);

    // Function to handle sprinting
    void Sprint(const FInputActionValue& Value);

    void Shoot(const FInputActionValue& Value);

    void ToggleShooting(bool bShouldShoot);

    // Function to handle the sprint logic
    void HandleSprint();

    // Function to handle the shooting logic
    void HandleShoot();

    // Function to rotate the player to the mouse cursor
    void RotateToMouse(float DeltaTime);

    // Function to get lifetime replicated properties
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void EquipWeapon();

private:
    // Rotation of the player
    FRotator PlayerRot;   

    // World position of the mouse
    FVector MouseWorldPosition;

    FRotator LastRotation;

    bool bIsShooting;

    float RotationInterpSpeed = 10.0f; // —корость интерпол€ции поворота

    float RotationUpdateInterval = 0.05f; // »нтервал обновлени€ поворота в секундах

    float TimeSinceLastRotationUpdate = 0.0f;

};