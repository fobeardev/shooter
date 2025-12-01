// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Abil_Interact.generated.h"

UCLASS()
class SHOOTER_API UAbil_Interact : public UGameplayAbility
{
    GENERATED_BODY()

public:

    UAbil_Interact();

protected:

    // Maximum distance for interaction line trace
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
    float MaxInteractDistance;

    // Collision channel used for the trace (usually ECC_Visibility or a custom channel)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
    TEnumAsByte<ECollisionChannel> TraceChannel;

    // Draw debug line for interaction traces
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
    bool bDebugTrace;

    // UGameplayAbility interface
    virtual bool CanActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags,
        const FGameplayTagContainer* TargetTags,
        FGameplayTagContainer* OptionalRelevantTags) const override;

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled) override;

private:

    // Performs the line trace and returns the first interactable actor, if any
    AActor* FindInteractableActor(const FGameplayAbilityActorInfo* ActorInfo, FHitResult& OutHit) const;
};