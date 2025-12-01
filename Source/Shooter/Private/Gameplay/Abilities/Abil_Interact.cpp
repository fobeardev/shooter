// Fill out your copyright notice in the Description page of Project Settings.


// Abil_Interact.cpp
// Plain ASCII only

#include "Gameplay/Abilities/Abil_Interact.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"

#include "Gameplay/Characters/Player/ShooterCharacter.h"
#include "Gameplay/Interaction/InteractableComponent.h"
#include "Gameplay/Tags/ShooterGameplayTags.h"
#include "Gameplay/Tags/ShooterGameplayTags_Interact.h"

UAbil_Interact::UAbil_Interact()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    MaxInteractDistance = 250.0f;
    bDebugTrace = false;

    SetAssetTags(FGameplayTagContainer(ShooterTags::Ability_Utility_Interact));
}

bool UAbil_Interact::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags,
    FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        return false;
    }

    AShooterCharacter* SC = Cast<AShooterCharacter>(ActorInfo->AvatarActor.Get());
    if (!SC)
    {
        return false;
    }

    FHitResult Hit;
    AActor* HitActor = SC->PerformInteractionTrace(MaxInteractDistance, Hit);
    if (!HitActor)
    {
        return false;
    }

    UInteractableComponent* Comp = HitActor->FindComponentByClass<UInteractableComponent>();
    return Comp && Comp->CanInteract(SC);
}

void UAbil_Interact::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    UE_LOG(LogTemp, Error, TEXT("GA_Interact ACTIVATED!"));

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AShooterCharacter* SC = Cast<AShooterCharacter>(ActorInfo->AvatarActor.Get());
    if (!SC)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Trace using the character
    FHitResult Hit;
    AActor* TargetActor = SC->PerformInteractionTrace(MaxInteractDistance, Hit);

    if (!TargetActor)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UInteractableComponent* Comp = TargetActor->FindComponentByClass<UInteractableComponent>();
    if (!Comp)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!Comp->CanInteract(SC))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Execute C++/BP event
    Comp->Interact(SC);

    // Send gameplay event for GAS listeners
    FGameplayEventData EventData;
    EventData.EventTag = ShooterTags::Event_Interact;
    EventData.Instigator = SC;
    EventData.Target = TargetActor;

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        TargetActor,
        ShooterTags::Event_Interact,
        EventData);

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UAbil_Interact::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
