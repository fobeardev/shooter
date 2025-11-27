// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AugmentData.h"
#include "Components/SphereComponent.h"
#include "Player/Components/InteractableComponent.h"
#include "AugmentPedestal.generated.h"

class UStaticMeshComponent;
class UInteractableComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FAugmentChosenDelegate,
    AAugmentPedestal*, Pedestal
);

UCLASS()
class SHOOTER_API AAugmentPedestal : public AActor
{
    GENERATED_BODY()

public:
    AAugmentPedestal();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* PedestalMesh;

    // NEW: Interaction component used by GA_Interact
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact")
    UInteractableComponent* InteractableComp;

    // Kept for designers if they want a visual bubble.
    // Not used for interact logic anymore.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Augment")
    FAugmentData AugmentData;

    bool bSelected = false;

public:

    /** Broadcast when this pedestal is chosen */
    UPROPERTY(BlueprintAssignable, Category = "Augment")
    FAugmentChosenDelegate OnAugmentChosen;

    /** Called by InteractableComponent->Interact() */
    UFUNCTION(BlueprintCallable)
    void HandleInteract(AActor* InteractingActor);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnAugmentChosen();
};
