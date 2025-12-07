#include "Gameplay/Combat/Projectile/ShooterProjectilePoolSubsystem.h"

#include "Gameplay/Combat/Projectile/ShooterProjectile.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UShooterProjectilePoolSubsystem::UShooterProjectilePoolSubsystem()
{
}

void UShooterProjectilePoolSubsystem::Deinitialize()
{
    InactiveProjectiles.Empty();
    ActiveProjectiles.Empty();

    Super::Deinitialize();
}

AShooterProjectile* UShooterProjectilePoolSubsystem::SpawnProjectile(const FProjectileSpawnParams& Params)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    if (World->GetNetMode() == NM_Client)
    {
        // Server only; clients receive replicated projectiles.
        return nullptr;
    }

    AShooterProjectile* Projectile = AcquireOrCreateProjectile(Params.Config);
    if (!Projectile)
    {
        return nullptr;
    }

    Projectile->OnPooledActivate(
        Params.SpawnLocation,
        Params.Direction,
        Params.InstigatorController,
        Params.InstigatorActor
    );

    ActiveProjectiles.Add(Projectile);

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

        if (!Projectile)
        {
            return nullptr;
        }

        Projectile->InitializeFromConfig(Config);
    }

    if (!Projectile)
    {
        return nullptr;
    }

    if (!Projectile->IsActive())
    {
        Projectile->InitializeFromConfig(Config);
    }

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
