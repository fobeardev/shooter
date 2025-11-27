// Fill out your copyright notice in the Description page of Project Settings.


#include "Augments/AugmentPedestal.h"

#include "Player/Components/InteractableComponent.h"
#include "Augments/AugmentManagerComponent.h"
#include "Characters/ShooterCharacter.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

AAugmentPedestal::AAugmentPedestal()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    PedestalMesh = CreateDefaultSubobject<UStaticMeshComponent>("PedestalMesh");
    SetRootComponent(PedestalMesh);

    // Optional design bubble
    InteractionSphere = CreateDefaultSubobject<USphereComponent>("InteractionSphere");
    InteractionSphere->SetupAttachment(PedestalMesh);
    InteractionSphere->SetSphereRadius(150.f);

    // NEW: Interactable component
    InteractableComp = CreateDefaultSubobject<UInteractableComponent>("InteractableComp");

    // Default display text for UI
    InteractableComp->InteractDisplayText = FText::FromString(TEXT("Choose Augment"));

    // Bind interaction
    InteractableComp->OnInteracted.AddDynamic(this, &AAugmentPedestal::HandleInteract);
}

void AAugmentPedestal::BeginPlay()
{
    Super::BeginPlay();
}

void AAugmentPedestal::HandleInteract(AActor* InteractingActor)
{
    if (bSelected)
    {
        return;
    }

    AShooterCharacter* Player = Cast<AShooterCharacter>(InteractingActor);
    if (!Player)
    {
        return;
    }

    UAugmentManagerComponent* AugMgr =
        Player->FindComponentByClass<UAugmentManagerComponent>();

    if (!AugMgr)
    {
        return;
    }

    if (HasAuthority())
    {
        // Apply augment
        AugMgr->ApplyAugment(AugmentData);

        bSelected = true;

        // Notify arena manager
        OnAugmentChosen.Broadcast(this);

        // Hide and disable for all clients
        Multicast_OnAugmentChosen();
    }
}

void AAugmentPedestal::Multicast_OnAugmentChosen_Implementation()
{
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    if (InteractableComp)
    {
        InteractableComp->SetInteractable(false);
    }
}
