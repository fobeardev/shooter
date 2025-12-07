#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ProjectileConfig.h"
#include "ShooterProjectilePoolSubsystem.generated.h"

class AShooterProjectile;

USTRUCT()
struct FProjectileSpawnParams
{
    GENERATED_BODY()

    UPROPERTY()
    FProjectileConfig Config;

    UPROPERTY()
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY()
    FVector Direction = FVector::ForwardVector;

    UPROPERTY()
    AController* InstigatorController = nullptr;

    UPROPERTY()
    AActor* InstigatorActor = nullptr;
};

UCLASS()
class SHOOTER_API UShooterProjectilePoolSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UShooterProjectilePoolSubsystem();

    virtual void Deinitialize() override;

    AShooterProjectile* SpawnProjectile(const FProjectileSpawnParams& Params);

protected:
    AShooterProjectile* AcquireOrCreateProjectile(const FProjectileConfig& Config);
    void ReturnToPool(AShooterProjectile* Projectile);

protected:
    UPROPERTY()
    TArray<TObjectPtr<AShooterProjectile>> InactiveProjectiles;

    UPROPERTY()
    TArray<TObjectPtr<AShooterProjectile>> ActiveProjectiles;

    UPROPERTY(EditAnywhere, Category = "Pool")
    int32 InitialPoolSize = 128;

    UPROPERTY(EditAnywhere, Category = "Pool")
    int32 MaxPoolSize = 1024;
};
