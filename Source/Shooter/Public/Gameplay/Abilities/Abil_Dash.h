#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "Abil_Dash.generated.h"

class UGameplayEffect;

UCLASS()
class SHOOTER_API UAbil_Dash : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UAbil_Dash();

    // Tags / Effects
    UPROPERTY(EditDefaultsOnly, Category = "GAS|Tags")
    FGameplayTag Tag_Ability_Dash;          // "Ability.Movement.Dash"

    UPROPERTY(EditDefaultsOnly, Category = "GAS|Tags")
    FGameplayTag Cue_DashStart;             // "GameplayCue.Dash.Start"

    UPROPERTY(EditDefaultsOnly, Category = "GAS|Effects")
    TSubclassOf<UGameplayEffect> GE_IFrames;

    // Tuning
    UPROPERTY(EditDefaultsOnly, Category = "Dash")
    float DashSpeed = 2200.f;

    UPROPERTY(EditDefaultsOnly, Category = "Dash")
    float DashDuration = 0.20f;

    UPROPERTY(EditDefaultsOnly, Category = "Dash")
    float IFrameDuration = 0.18f;

    UPROPERTY(EditDefaultsOnly, Category = "Dash|Air")
    bool bAllowAirDash = true;

    UPROPERTY(EditDefaultsOnly, Category = "Dash|Air", meta = (EditCondition = "bAllowAirDash"))
    float AirDashSpeedMultiplier = 0.75f;

    UPROPERTY(EditDefaultsOnly, Category = "Dash|Air", meta = (EditCondition = "bAllowAirDash"))
    float AirDashFallingLateralFriction = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Dash|Air", meta = (EditCondition = "bAllowAirDash"))
    float AirDashBrakingDecel = 600.f;

    UPROPERTY(EditDefaultsOnly, Category = "Dash|Air", meta = (EditCondition = "bAllowAirDash"))
    float AirControlDuringAirDash = 0.25f;

    UPROPERTY(EditDefaultsOnly, Category = "Dash|Air", meta = (EditCondition = "bAllowAirDash"))
    float MaxUpwardVelDuringAirDash = 0.0f;

    // Charge vs cooldown (charges expected on character)
    UPROPERTY(EditDefaultsOnly, Category = "Dash|Activation")
    bool bUseCharges = true;

    // Optional flat cooldown GE (only if bUseCharges=false)
    UPROPERTY(EditDefaultsOnly, Category = "GAS|Effects")
    TSubclassOf<UGameplayEffect> GE_Cooldown;

protected:
    // GAS
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags,
        const FGameplayTagContainer* TargetTags,
        OUT FGameplayTagContainer* OptionalRelevantTags) const override;

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility, bool bWasCancelled) override;

    virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilitySpec& Spec) override;

    virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilitySpec& Spec) override;

private:
    // Saved movement to restore
    float SavedBrakingFriction = -1.f;
    float SavedFallingLateralFriction = -1.f;
    float SavedBrakingDecelFalling = -1.f;
    float SavedAirControl = -1.f;

    FTimerHandle EndTimer;

    void EndDash();
    UWorld* GetSafeWorld(const FGameplayAbilityActorInfo* ActorInfo) const;

    // --- Chain / spam control (time-based, no tags)
    UPROPERTY(EditDefaultsOnly, Category = "Dash|Chain")
    float ChainUnlockDelay = 0.05f;   // was 0.08f; 0.05–0.07 feels snappy like Hades

    UPROPERTY(EditDefaultsOnly, Category = "Dash|Chain")
    float MinRetriggerGap = 0.03f;    // keep small to block same-frame double-fires

    // Is a dash currently active (between Activate and EndAbility)?
    mutable bool bDashActive = false;

    // When the last dash started
    mutable double LastDashStartTime = -1000.0;

    // Number of dashes triggered during the current active window (cap at 2 total)
    mutable int32 DashesThisWindow = 0;
};
