#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Actors/SKGBaseActor.h"                // SKG base
#include "ShooterWeaponBase.generated.h"

class UMeshComponent;
class USKGShooterPawnComponent;
class USKGProceduralAnimComponent;

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
	AShooterWeaponBase();

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

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void HandleFire_Internal();        // default: no-op
	virtual void HandleStopFire_Internal();    // default: no-op

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Weapon")
	TObjectPtr<UMeshComponent> WeaponMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Weapon")
	TObjectPtr<USKGProceduralAnimComponent> ProceduralAnimComponent;

	// Owner shooter pawn component (cached; NOT replicated)
	UPROPERTY()
	TObjectPtr<USKGShooterPawnComponent> ShooterPawn = nullptr;

	/** Optional data asset pointer for game-side tuning; NOT replicated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooter|Weapon")
	TObjectPtr<UPrimaryDataAsset> WeaponDataAsset = nullptr;

	/** Weapon class (Pistol/AR/etc) for UI/rules. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooter|Weapon|Tags")
	FGameplayTag WeaponClassTag;

	/** What type of damage this weapon deals (Ballistic/Energy/etc). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooter|Weapon|Tags")
	FGameplayTag DamageTypeTag;

	/** Replicated fire mode (Semi/Burst/Auto). */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Shooter|Weapon|Tags")
	FGameplayTag CurrentFireModeTag;

	/** Replicated “busy” bit used by CanPerformAction. */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Shooter|Weapon|State")
	bool bIsReloading = false;

public:

	void SetShooterPawn(USKGShooterPawnComponent* InPawn)
	{
		ShooterPawn = InPawn;
	}

	FORCEINLINE USKGShooterPawnComponent* GetShooterPawn() const
	{
		return ShooterPawn;
	}
};
