// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

///////////////////////////////////////////////////////////////////////////////////////////////////
// Sets default values
APlayerCharacter::APlayerCharacter()
{   

    // Enable replication
    bIsShooting = false;
    bReplicates = true;
    SetReplicateMovement(true);

    // Set this character to call Tick() every frame. You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

    // Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
    // instead of recompiling to adjust them
    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 300.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
}
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = Cast<APlayerController>(GetController());

    //Set show mouse and click events
    if (PC)
    {
        PC->bShowMouseCursor = true;
        PC->bEnableClickEvents = true;
        PC->bEnableMouseOverEvents = true;
    }

    // Add Input Mapping Context
    if (PC)
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    EquipWeapon();

}
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
void APlayerCharacter::EquipWeapon()
{
    if (WeaponClass)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            //Spawn and attach weapon to player
            if (HasAuthority())
            {
                CurrentWeapon = World->SpawnActor<AWeaponBase>(WeaponClass);
                if (CurrentWeapon)
                {
                    CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("RightHand"));
                    CurrentWeapon->SetActorRelativeRotation(WeaponRotation);
                    CurrentWeapon->SetActorRelativeLocation(WeaponLocation);
                }
            }
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
        // Moving
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

        // Sprinting
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &APlayerCharacter::Sprint);
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APlayerCharacter::Sprint);
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &APlayerCharacter::Sprint);

        //Shooting
        EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Shoot);
        EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Canceled, this, &APlayerCharacter::Shoot);
        EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &APlayerCharacter::Shoot);
    }
    else
    {
        UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void APlayerCharacter::Move(const FInputActionValue& Value)
{
    // input is a Vector2D
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // get forward vector
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

        // get right vector 
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // add movement 
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
//Sprint func call on client and server
void APlayerCharacter::Sprint(const FInputActionValue& Value)
{
    if (HasAuthority())
    {
        HandleSprint();
    }
    else
    {
        //Client prediction
        HandleSprint();
        ServerSprint();
    }
}

void APlayerCharacter::ServerSprint_Implementation()
{
    HandleSprint();
}

bool APlayerCharacter::ServerSprint_Validate()
{
    return true;
}

//Sprint func
void APlayerCharacter::HandleSprint()
{
    float Speed = GetCharacterMovement()->MaxWalkSpeed;
    const float NormalSpeed = 300.0f;
    const float MaxSpeed = 500.0f;

    if (Speed <= NormalSpeed)
    {
        GetCharacterMovement()->MaxWalkSpeed = MaxSpeed;
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
    }  
}
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
//Check if player can shoot
void APlayerCharacter::Shoot(const FInputActionValue& Value)
{
    bool bShouldShoot = Value.Get<bool>();
    ToggleShooting(bShouldShoot);
}

//Call client and server shoot func according to bShouldShoot and bIsShooting
void APlayerCharacter::ToggleShooting(bool bShouldShoot)
{
    if (HasAuthority())
    {
        if (bShouldShoot && !bIsShooting)
        {
            bIsShooting = true;
            HandleShoot();
            GetWorldTimerManager().SetTimer(ShootTimerHandle, this, &APlayerCharacter::HandleShoot, 0.1f, true);
        }
        else if (!bShouldShoot && bIsShooting)
        {
            bIsShooting = false;
            GetWorldTimerManager().ClearTimer(ShootTimerHandle);
        }
    }
    else
    {
        ServerShoot(bShouldShoot);
    }
}

void APlayerCharacter::ServerShoot_Implementation(bool bShouldShoot)
{
    ToggleShooting(bShouldShoot);
}

bool APlayerCharacter::ServerShoot_Validate(bool bShouldShoot)
{
    return true;
}

//Main shoot func
void APlayerCharacter::HandleShoot()
{  
    ShootDirection = GetActorForwardVector();
    FVector ProjectileOffset = GetActorLocation() + ShootDirection * 150 + FVector(0, 0, 50);
    if (CurrentWeapon)
    {
        CurrentWeapon->ShootBullet(rot);
    }
       
}
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Rotate player body to mouse cursor
void APlayerCharacter::RotateToMouse(float DeltaTime)
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());

    if (PlayerController && PlayerController->IsLocalController())
    {
        FVector2D MousePosition;
        if (PlayerController->GetMousePosition(MousePosition.X, MousePosition.Y))
        {
            // Convert mouse 2d coord to 3d and get mouse world position
            FVector WorldLocation, WorldDirection;
            PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);
            MouseWorldPosition = FMath::LinePlaneIntersection(WorldLocation, WorldLocation + WorldDirection * 10000.f, GetActorLocation(), GetActorUpVector());

            PlayerRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), MouseWorldPosition);
            PlayerRot.Yaw = UKismetMathLibrary::NormalizeAxis(PlayerRot.Yaw);

            if (PlayerRot != rot)
            {
                if (HasAuthority())
                {
                    rot = PlayerRot;
                    MulticastSetActorRotation(rot);
                    if (IsLocallyControlled())
                    {
                        FRotator CurrentRotation = GetActorRotation();
                        FRotator InterpolatedRotation = FMath::RInterpTo(CurrentRotation, rot, DeltaTime, RotationInterpSpeed);
                        SetActorRotation(InterpolatedRotation);
                    }
                }
                else
                {
                    rot = PlayerRot;
                    ServerRotateToMouse(rot);

                    //Client prediction
                    FRotator CurrentRotation = GetActorRotation();
                    FRotator InterpolatedRotation = FMath::RInterpTo(CurrentRotation, rot, DeltaTime, RotationInterpSpeed);
                    SetActorRotation(InterpolatedRotation);
                }
            }
        }
    }
}

void APlayerCharacter::ServerRotateToMouse_Implementation(FRotator NewRotation)
{
    rot = NewRotation;
    MulticastSetActorRotation(rot);
}

bool APlayerCharacter::ServerRotateToMouse_Validate(FRotator NewRotation)
{
    return true;
}

void APlayerCharacter::MulticastSetActorRotation_Implementation(FRotator NewRotation)
{
    if (!IsLocallyControlled())
    {
        //Set rotation on every client except local
        rot = NewRotation;
        SetActorRotation(rot);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    RotateToMouse(DeltaTime);

    if (!HasAuthority())
    {
        FRotator CurrentRotation = GetActorRotation();
        FRotator InterpolatedRotation = FMath::RInterpTo(CurrentRotation, rot, DeltaTime, RotationInterpSpeed);
        SetActorRotation(InterpolatedRotation);
    }
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APlayerCharacter, rot);
    DOREPLIFETIME(APlayerCharacter, ShootDirection);
}