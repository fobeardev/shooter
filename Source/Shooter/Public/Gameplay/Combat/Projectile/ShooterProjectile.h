#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileConfig.h"
#include "ShooterProjectile.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class SHOOTER_API AShooterProjectile : public AActor
{
    GENERATED_BODY()

public:
    AShooterProjectile();

    virtual void Tick(float DeltaSeconds) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Called by pool subsystem to initialize before activation
    void InitializeFromConfig(const FProjectileConfig& InConfig);

    // Pool API
    void OnPooledActivate(const FVector& SpawnLocation, const FVector& Direction, AController* InstigatorController, AActor* InstigatorActor);
    void OnPooledDeactivate();

    bool IsActive() const { return bIsActive; }

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnProjectileHit(const FHitResult& Hit);

    void ApplyDamage(const FHitResult& Hit);
    void SpawnImpactFX(const FHitResult& Hit);
    void UpdateTracerFX();

protected:
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

    // Not replicated; config should be identical across clients
    FProjectileConfig Config;

    TWeakObjectPtr<AController> InstigatorControllerWeak;
    TWeakObjectPtr<AActor> InstigatorActorWeak;
};
