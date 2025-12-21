#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Actors/SKGBaseActor.h" // SKG base

#include "ShooterWeaponBase.generated.h"

// Forward declarations
class UMeshComponent;
class USKGShooterPawnComponent;
class USKGProceduralAnimComponent;

struct FProjectileConfig;
struct FProjectileIdentity;

/**
 * Shared parent for all weapons (firearms, melee, energy).
 * - Replicates core weapon state (fire mode, reloading)
 * - Exposes mesh access and generic action gates
 * - Hosts GAS-/input-facing entry points (virtual)
 */
UCLASS(Abstract)
class SHOOTER_API AShooterWeaponBase : public ASKGBaseActor
{
    GENERATED_BODY()

public:
    // Constructor
    AShooterWeaponBase();

    UFUNCTION(BlueprintCallable, Category = "Shooter|Weapon")
    virtual void FireWithProjectileSpec(
        const FProjectileConfig& Config,
        const FProjectileIdentity& Identity
    );

    UFUNCTION(Server, Reliable)
    void Server_FireWithProjectileSpec(
        const FProjectileConfig& Config,
        const FProjectileIdentity& Identity
    );

    // --- Core virtuals for children ---
    UFUNCTION(BlueprintCallable, Category = "Shooter|Weapon")
    virtual void Fire();

    UFUNCTION(Server, Reliable)
    virtual void Server_Fire();

    UFUNCTION(BlueprintCallable, Category = "Shooter|Weapon")
    virtual void StopFire();

    UFUNCTION(Server, Reliable)
    virtual void Server_StopFire();

    /** Default “can I act now?” gate shared by children. */
    UFUNCTION(BlueprintPure, Category = "Shooter|Weapon")
    virtual bool CanPerformAction() const;

    /** The primary mesh of this weapon (child overrides if needed). */
    UFUNCTION(BlueprintPure, Category = "Shooter|Weapon")
    virtual UMeshComponent* GetWeaponMesh() const;

    /** Current fire mode (replicated). */
    UFUNCTION(BlueprintPure, Category = "Shooter|Weapon")
    const FGameplayTag& GetCurrentFireModeTag() const { return CurrentFireModeTag; }

    // Public setters and getters
    void SetShooterPawn(USKGShooterPawnComponent* InPawn)
    {
        ShooterPawn = InPawn;
    }

    FORCEINLINE USKGShooterPawnComponent* GetShooterPawn() const
    {
        return ShooterPawn;
    }

protected:
    // Overrides
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Internal handlers
    virtual void HandleFire_Internal();        // default: no-op
    virtual void HandleStopFire_Internal();    // default: no-op

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Weapon")
    TObjectPtr<UMeshComponent> WeaponMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Weapon")
    TObjectPtr<USKGProceduralAnimComponent> ProceduralAnimComponent;

    // Cached owner shooter pawn component (NOT replicated)
    UPROPERTY()
    TObjectPtr<USKGShooterPawnComponent> ShooterPawn = nullptr;

    // Weapon configuration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooter|Weapon")
    TObjectPtr<UPrimaryDataAsset> WeaponDataAsset = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooter|Weapon|Tags")
    FGameplayTag WeaponClassTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooter|Weapon|Tags")
    FGameplayTag DamageTypeTag;

    // Replicated state
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Shooter|Weapon|Tags")
    FGameplayTag CurrentFireModeTag;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Shooter|Weapon|State")
    bool bIsReloading = false;
};