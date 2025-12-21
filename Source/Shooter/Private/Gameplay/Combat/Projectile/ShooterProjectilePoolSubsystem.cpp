#include "Gameplay/Combat/Projectile/ShooterProjectilePoolSubsystem.h"
#include "Gameplay/Combat/Projectile/ShooterProjectile.h"
#include "Engine/World.h"

UShooterProjectilePoolSubsystem::UShooterProjectilePoolSubsystem()
{
}

void UShooterProjectilePoolSubsystem::Deinitialize()
{
    InactiveProjectiles.Empty();
    ActiveProjectiles.Empty();

    Super::Deinitialize();
}

AShooterProjectile* UShooterProjectilePoolSubsystem::SpawnProjectile(
    TSubclassOf<AActor> ProjectileClass,
    const FVector& SpawnLocation,
    const FVector& ShootDirection,
    const FProjectileConfig& Config,
    const FProjectileIdentity& Identity,
    AActor* InstigatorActor,
    AController* InstigatorController)
{
    if (!GetWorld() || !ProjectileClass)
    {
        return nullptr;
    }

    AShooterProjectile* Projectile = AcquireFromPool(ProjectileClass); // Your existing pool acquire
    if (!Projectile)
    {
        // Fallback spawn if pool is empty (still valid)
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Params.Owner = InstigatorActor;
        Params.Instigator = Cast<APawn>(InstigatorActor);

        Projectile = GetWorld()->SpawnActor<AShooterProjectile>(ProjectileClass, SpawnLocation, ShootDirection.Rotation(), Params);
    }

    if (!Projectile)
    {
        return nullptr;
    }

    Projectile->InitializeFromSpec(
        SpawnLocation,
        ShootDirection,
        Config,
        Identity,
        InstigatorActor,
        InstigatorController,
        this
    );

    return Projectile;
}

AShooterProjectile* UShooterProjectilePoolSubsystem::AcquireOrCreateProjectile(const FProjectileConfig& Config)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    AShooterProjectile* Projectile = nullptr;

    if (InactiveProjectiles.Num() > 0)
    {
        Projectile = InactiveProjectiles.Pop();
    }
    else if (ActiveProjectiles.Num() + InactiveProjectiles.Num() < MaxPoolSize)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        Projectile = World->SpawnActor<AShooterProjectile>(
            AShooterProjectile::StaticClass(),
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            SpawnParams
        );
    }

    if (!Projectile)
    {
        return nullptr;
    }

    Projectile->InitializeFromConfig(Config);
    Projectile->SetOwningPool(this);

    return Projectile;
}

void UShooterProjectilePoolSubsystem::ReturnToPool(AShooterProjectile* Projectile)
{
    if (!Projectile)
    {
        return;
    }

    ActiveProjectiles.Remove(Projectile);
    InactiveProjectiles.Add(Projectile);
}
