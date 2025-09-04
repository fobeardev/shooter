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
 * Custom firearm class extending SKGFirearm with firing logic.
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Mode")
    FGameplayTagContainer FireModes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Mode")
    FGameplayTag CurrentFireMode;

    // -------------------- Full Auto --------------------
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Full Auto")
    FTimerHandle FullAutoTimerHandle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Full Auto", meta = (ClampMin = "0.0"))
    float FireRateDelay = 0.10f;

    // -------------------- Reload --------------------
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Reload")
    uint8 ReloadCounter = 0;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Reload")
    bool bIsReloading = false;

    // -------------------- Sockets --------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets")
    FTriggerData TriggerData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets")
    int32 DamagingProjectileId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets")
    FTransform StartingTransform = FTransform::Identity;

    UPROPERTY(ReplicatedUsing = OnRep_HasOwner, VisibleAnywhere, BlueprintReadOnly, Category = "Sockets")
    bool bHasOwner = false;

    // ---------- Blueprint → C++ events ----------
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable) void SetOwningPawn(APawn* OwningPawn);
    UFUNCTION(BlueprintCallable) void Interact(APawn* InteractingPawn);
    UFUNCTION(BlueprintCallable) void Drop();

    UFUNCTION(BlueprintCallable) void SetupFireModes(const FGameplayTagContainer& FireModeTags);
    UFUNCTION(BlueprintCallable) void SetFireRateDelay(double FireRate);

    UFUNCTION()
    void OnDAConstructionBundlesLoaded();

    // --- Shooting (input → server → counters) ---
    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void PressTrigger();

    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void ReleaseTrigger();

protected:
    UFUNCTION()
    void OnRep_HasOwner();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Shooting
    // Server RPCs
    UFUNCTION(Server, Reliable)
    void Server_PressTrigger();

    UFUNCTION(Server, Reliable)
    void Server_ReleaseTrigger();

    UFUNCTION(Server, Reliable)
    void Server_LaunchProjectile(const FSKGMuzzleTransform& LaunchTransform);

    // Blueprint helpers
    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void IncreasePressTrigger();

    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void IncreaseReleaseTrigger();

    UFUNCTION(BlueprintPure, Category = "Shooting")
    bool CanPerformAction() const;

    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void Fire();

    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void StopFire();

    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void PerformFireAnimation();

    // Events for Blueprint
    UFUNCTION(BlueprintImplementableEvent, Category = "Shooting")
    void PlayFireEffects();

    UFUNCTION(BlueprintImplementableEvent, Category = "Shooting")
    void LaunchProjectile(const FSKGMuzzleTransform& MuzzleTransform, bool bIsLocalFire);

    UFUNCTION(BlueprintPure, Category = "Shooting")
    bool IsDamagingProjectile(int32 ProjectileId) const;
#pragma endregion
};
