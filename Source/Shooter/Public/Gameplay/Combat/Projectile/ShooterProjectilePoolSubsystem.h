#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Gameplay/Combat/Projectile/ProjectileConfig.h"
#include "Gameplay/Combat/Projectile/ProjectileIdentity.h"
#include "ShooterProjectilePoolSubsystem.generated.h"

class AShooterProjectile;

UCLASS()
class SHOOTER_API UShooterProjectilePoolSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UShooterProjectilePoolSubsystem();

    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Shooter|Projectile")
    class AShooterProjectile* SpawnProjectile(
        TSubclassOf<AActor> ProjectileClass,
        const FVector& SpawnLocation,
        const FVector& ShootDirection,
        const FProjectileConfig& Config,
        const FProjectileIdentity& Identity,
        AActor* InstigatorActor,
        AController* InstigatorController);

    void ReturnToPool(AShooterProjectile* Projectile);

protected:
    AShooterProjectile* AcquireOrCreateProjectile(const FProjectileConfig& Config);

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
