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
    const FVector& SpawnLocation,
    const FVector& Direction,
    const FProjectileConfig& Config,
    const FProjectileIdentity& Identity,
    AActor* InstigatorActor,
    AController* InstigatorController)
{
    if (!GetWorld() || !DefaultProjectileClass)
    {
        return nullptr;
    }

    AShooterProjectile* Projectile = GetOrCreateProjectile(DefaultProjectileClass);
    if (!Projectile)
    {
        return nullptr;
    }

    Projectile->InitializeFromSpec(
        SpawnLocation,
        Direction,
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
