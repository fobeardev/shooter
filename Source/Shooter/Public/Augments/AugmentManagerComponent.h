// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AugmentData.h"
#include "AugmentManagerComponent.generated.h"

class UAbilitySystemComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SHOOTER_API UAugmentManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAugmentManagerComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY()
    UAbilitySystemComponent* ASC;

    UPROPERTY(ReplicatedUsing = OnRep_AppliedAugments)
    TArray<FAugmentData> AppliedAugments;

    UFUNCTION()
    void OnRep_AppliedAugments();

public:
    UFUNCTION(BlueprintCallable)
    void ApplyAugment(const FAugmentData& Data);

    UFUNCTION(BlueprintCallable)
    void ApplyGEToSelf(TSubclassOf<UGameplayEffect> GEClass);
};
