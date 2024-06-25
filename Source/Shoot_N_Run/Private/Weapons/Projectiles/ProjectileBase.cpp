// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Projectiles/ProjectileBase.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AProjectileBase::AProjectileBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;

    if (!RootComponent)
    {
        RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSceneComponent"));
    }

    if (!CollisionComponent)
    {
        
        CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
        
        CollisionComponent->InitSphereRadius(15.0f);
        
        RootComponent = CollisionComponent;
    }

    if (!ProjectileMovementComponent)
    {
        // Use this component to drive this projectile's movement.
        ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
        ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
        ProjectileMovementComponent->InitialSpeed = 4000.0f;
        ProjectileMovementComponent->MaxSpeed = 4000.0f;
        ProjectileMovementComponent->bRotationFollowsVelocity = true;
        ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
    }

    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::BeginOverlap);

    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECollisionResponse::ECR_Overlap);
}

void AProjectileBase::BeginOverlap(UPrimitiveComponent* OverlappedComponent, 
                                    AActor* OtherActor, 
                                    UPrimitiveComponent* OtherComp, 
                                    int32 OtherBodyIndex, 
                                    bool bFromSweep, const 
                                    FHitResult& SweepResult)
{
    if (OverlappedComponent == CollisionComponent)
    {       
        APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);   
        if (Player != nullptr)
        {
            if (HasAuthority())
            {
                ServerDestroyPlayer(Player);
            }
        }
        else
            Destroy();
    }
}

void AProjectileBase::ServerDestroyPlayer_Implementation(APlayerCharacter* Player)
{
    if (Player != nullptr)
    {
        // ”ничтожаем текущее оружие игрока
        if (Player->CurrentWeapon)
        {
            AWeaponBase* Weapon = Player->CurrentWeapon;
            if (Weapon)
            {
                Weapon->Destroy();
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Weapon was nullptr after cast"));
            }
        }

        // ”ничтожаем самого игрока
        Player->Destroy();

        // ”ничтожаем сам снар€д
        Destroy();
    }
}

bool AProjectileBase::ServerDestroyPlayer_Validate(APlayerCharacter* Player)
{
    return true;
}



void AProjectileBase::FireInDirection(const FVector& ShootDirection)
{
    if (HasAuthority())
    {
        HandleFireInDirection(ShootDirection);
    }
    else
    {
        ServerFireInDirection(ShootDirection);
    }
}

void AProjectileBase::ServerFireInDirection_Implementation(const FVector& ShootDirection)
{
    HandleFireInDirection(ShootDirection);
}

bool AProjectileBase::ServerFireInDirection_Validate(const FVector& ShootDirection)
{
    return true;
}

void AProjectileBase::HandleFireInDirection(const FVector& ShootDirection)
{
    ProjectileMovementComponent->Velocity = ShootDirection * ProjectileMovementComponent->InitialSpeed;
}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

