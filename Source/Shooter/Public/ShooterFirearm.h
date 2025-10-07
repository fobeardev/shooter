// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Actors/SKGFirearm.h"
#include "TriggerData.h"
#include "ShooterFirearm.generated.h"

class USKGShooterPawnComponent;
struct FSKGMuzzleTransform;

/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterFirearm : public ASKGFirearm
{
	GENERATED_BODY()

public:
    AShooterFirearm();

	// -------------------- Components / References --------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
    TObjectPtr<USKGShooterPawnComponent> CurrentShooterPawnComponent = nullptr;

    // -------------------- Fire Mode --------------------
    // BP: FireModes (Gameplay Tag array icon) — if yours is an array, use container; if it was a single tag array, keep TArray<FGameplayTag>.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Firearm", meta = (AllowPrivateAccess = "true"))
    FGameplayTagContainer FireModes;

    // BP: CurrentFireMode (Gameplay Tag)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Firearm", meta = (AllowPrivateAccess = "true"))
    FGameplayTag CurrentFireMode;

    // -------------------- Full Auto --------------------
    // BP: FullAutoTimerHandle (Timer Handle)
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Full Auto")
    FTimerHandle FullAutoTimerHandle;

    // BP: FireRateDelay (Float)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Firearm", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
    float FireRateDelay = 0.10f;


    // -------------------- Reload --------------------
    // BP: ReloadCounter (Byte)
    // Reload
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Reload")
    uint8 ReloadCounter = 0;

    // BP: bIsReloading (Boolean)
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Reload")
    bool bIsReloading = false;

    // -------------------- Sockets --------------------
    // BP: TriggerData (S Trigger Data) — already converted
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets")
    FTriggerData TriggerData;

    // BP: DamagingProjectileId (Integer)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets")
    int32 DamagingProjectileId = 0;

    // BP: StartingTransform (Transform)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets")
    FTransform StartingTransform = FTransform::Identity;

    // BP: bHasOwner (Boolean)
    UPROPERTY(ReplicatedUsing = OnRep_HasOwner, VisibleAnywhere, BlueprintReadOnly, Category = "Sockets")
    bool bHasOwner = false;

    // ---------- Blueprint → C++ events ----------
    virtual void BeginPlay() override;

    // From BP interface events:
    UFUNCTION(BlueprintCallable) void SetOwningPawn(APawn* OwningPawn);
    UFUNCTION(BlueprintCallable) void Interact(APawn* InteractingPawn);
    UFUNCTION(BlueprintCallable) void Drop();

    // From Event Graph custom calls
    UFUNCTION(BlueprintCallable) void SetupFireModes(const FGameplayTagContainer& FireModeTags);
    UFUNCTION(BlueprintCallable) void SetFireRateDelay(double FireRate);

    // SKG "construction bundles loaded" (BP overrides this)
    UFUNCTION()
    void OnDAConstructionBundlesLoaded();

    // --- Shooting (input → server → counters) ---
    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void PressTrigger();

    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void ReleaseTrigger();

    UFUNCTION(BlueprintPure, Category = "Shooting")
    bool IsDamagingProjectile(int32 ProjectileId) const;

protected:
    UFUNCTION()
    void OnRep_HasOwner();

    // replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Shooting

    // Server RPCs that actually change state on the server
    UFUNCTION(Server, Reliable)
    void Server_PressTrigger();

    UFUNCTION(Server, Reliable)
    void Server_ReleaseTrigger();

    // Small helpers (match the BP names so it's easy to follow)
    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void IncreasePressTrigger();

    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void IncreaseReleaseTrigger();

    // Gate like the BP "CanPerformAction" node
    UFUNCTION(BlueprintPure, Category = "Shooting")
    bool CanPerformAction() const;

    // (Optional stubs to mirror the BP list; fill these out later)
    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void Fire();

    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void StopFire();

    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void PerformFireAnimation();
#pragma endregion

#pragma region Projectile

    UFUNCTION(BlueprintImplementableEvent, Category = "Shooting")
    void PlayFireEffects();

    UFUNCTION(BlueprintImplementableEvent, Category = "Shooting")
    void LaunchProjectile(const FSKGMuzzleTransform& MuzzleTransform, bool bIsLocalFire);

    UFUNCTION(Server, Reliable)
    void Server_LaunchProjectile(const FSKGMuzzleTransform& LaunchTransform);

#pragma endregion

    // -------------------- Fire Mode (BP parity) --------------------

	// BP: CycleFireMode (Interface override path)
    UFUNCTION(BlueprintCallable, Category = "Fire Mode")
    void CycleFireMode();

    // Server RPC that does the authoritative write
    UFUNCTION(Server, Reliable)
    void Server_CycleFireMode();

    // BP: GetNextFireMode (Pure)
    UFUNCTION(BlueprintPure, Category = "Fire Mode")
    FGameplayTag GetNextFireMode() const;

    // --- Reloading ---

// Client entry (from BPI_Firearm: Reload)
    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void Reload();

    // Server authoritative gate
    UFUNCTION(Server, Reliable)
    void Server_Reload();

    // Plays the two reloading montages (player + firearm)
    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void PerformReloadAnimations();

    // BP node replacement that finds the two montages to play
    UFUNCTION(BlueprintPure, Category = "Shooting")
    void GetReloadAnimations(/*out*/ UAnimMontage*& Firearm, /*out*/ UAnimMontage*& Player, /*out*/ bool& bValid) const;

    // Optional: called when a reload montage blends out to unset state
    UFUNCTION()
    void OnReloadMontageBlendOut(UAnimMontage* Montage, bool bInterrupted);

};
