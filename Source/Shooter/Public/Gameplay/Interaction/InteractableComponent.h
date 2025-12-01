// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

UENUM(BlueprintType)
enum class EInteractType : uint8
{
    None UMETA(DisplayName = "None"),
    Instant UMETA(DisplayName = "Instant"),
    Hold UMETA(DisplayName = "Hold")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractSignature, AActor*, InteractingActor);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SHOOTER_API UInteractableComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UInteractableComponent();

protected:

    virtual void BeginPlay() override;

public:

    // Whether this object can currently be interacted with
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    bool bIsInteractable;

    // Display text for UI prompts (example: "Open", "Pick Up", "Activate")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    FText InteractDisplayText;

    // The type of interaction (instant or hold)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    EInteractType InteractType;

    // How long a hold interaction requires, if applicable
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact", meta = (EditCondition = "InteractType == EInteractType::Hold", ClampMin = "0.1"))
    float HoldDuration;

    // Fires when interaction succeeds
    UPROPERTY(BlueprintAssignable, Category = "Interact")
    FOnInteractSignature OnInteracted;

    // Checks whether interaction is currently allowed
    UFUNCTION(BlueprintCallable, Category = "Interact")
    virtual bool CanInteract(AActor* InteractingActor) const;

    // Called by the Interact ability when interacting
    UFUNCTION(BlueprintCallable, Category = "Interact")
    virtual bool Interact(AActor* InteractingActor);

    // Enables or disables interaction at runtime
    UFUNCTION(BlueprintCallable, Category = "Interact")
    void SetInteractable(bool bNewState);
};
