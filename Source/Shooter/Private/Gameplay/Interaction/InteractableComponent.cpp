// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Interaction/InteractableComponent.h"

UInteractableComponent::UInteractableComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    bIsInteractable = true;
    InteractType = EInteractType::Instant;
    HoldDuration = 0.5f;

    InteractDisplayText = FText::FromString(TEXT("Interact"));
}

void UInteractableComponent::BeginPlay()
{
    Super::BeginPlay();
}

bool UInteractableComponent::CanInteract(AActor* InteractingActor) const
{
    if (!bIsInteractable)
    {
        return false;
    }

    if (InteractingActor == nullptr)
    {
        return false;
    }

    return true;
}

bool UInteractableComponent::Interact(AActor* InteractingActor)
{
    if (!CanInteract(InteractingActor))
    {
        return false;
    }

    // Broadcast BP event or C++ binding
    OnInteracted.Broadcast(InteractingActor);

    return true;
}

void UInteractableComponent::SetInteractable(bool bNewState)
{
    bIsInteractable = bNewState;
}
