#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Combat/Projectile/ProjectileConfig.h"
#include "Gameplay/Combat/Projectile/ProjectileIdentity.h"
#include "ShooterProjectile.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UShooterProjectilePoolSubsystem;

UCLASS()
class SHOOTER_API AShooterProjectile : public AActor
{
    GENERATED_BODY()

public:
    AShooterProjectile();

    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void InitializeFromSpec(
        const FVector& SpawnLocation,
        const FVector& ShootDirection,
        const FProjectileConfig& InConfig,
        const FProjectileIdentity& InIdentity,
        AActor* InInstigatorActor,
        AController* InInstigatorController,
        class UShooterProjectilePoolSubsystem* InOwningPool);
    
    // Pool API
    void OnPooledActivate(const FVector& SpawnLocation, const FVector& Direction, AController* InstigatorController, AActor* InstigatorActor);
    void OnPooledDeactivate();

    bool IsActive() const { return bIsActive; }

    void SetOwningPool(UShooterProjectilePoolSubsystem* InPool) { OwningPool = InPool; }

protected:
    virtual void BeginPlay() override;

    void HandleProjectileHit(const FHitResult& Hit);
    void ApplyDamage(const FHitResult& Hit);
    void SpawnImpactFX(const FHitResult& Hit);
    void UpdateTracerFX();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    FProjectileConfig Config;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    FProjectileIdentity Identity;

    UPROPERTY()
    TWeakObjectPtr<AActor> InstigatorActorWeak;

    UPROPERTY()
    TWeakObjectPtr<AController> InstigatorControllerWeak;

    UPROPERTY()
    TObjectPtr<UShooterProjectilePoolSubsystem> OwningPool;

    int32 RemainingRicochets = 0;
    int32 RemainingPierces = 0;
    
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USphereComponent* CollisionComponent;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UNiagaraComponent* TracerComponent;

    UPROPERTY(Replicated)
    FVector_NetQuantize Velocity;

    UPROPERTY(Replicated)
    FVector_NetQuantize10 StartLocation;

    UPROPERTY(Replicated)
    float ElapsedLifeTime;

    UPROPERTY(Replicated)
    bool bIsActive;
};
