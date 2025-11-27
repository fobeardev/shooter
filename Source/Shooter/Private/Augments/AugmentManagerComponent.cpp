// Fill out your copyright notice in the Description page of Project Settings.


#include "Augments/AugmentManagerComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Characters/ShooterCharacter.h"

UAugmentManagerComponent::UAugmentManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UAugmentManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    AShooterCharacter* OwnerChar = Cast<AShooterCharacter>(GetOwner());
    if (OwnerChar)
    {
        ASC = OwnerChar->GetAbilitySystemComponent();
    }
}

void UAugmentManagerComponent::ApplyAugment(const FAugmentData& Data)
{
    if (!ASC || !Data.GameplayEffectClass) return;

    ApplyGEToSelf(Data.GameplayEffectClass);
    AppliedAugments.Add(Data);

    // Replicate to clients
    if (GetOwnerRole() == ROLE_Authority)
    {
        OnRep_AppliedAugments();
    }
}

void UAugmentManagerComponent::ApplyGEToSelf(TSubclassOf<UGameplayEffect> GEClass)
{
    if (!ASC || !GEClass) return;

    FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
    ContextHandle.AddSourceObject(this);

    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GEClass, 1.0f, ContextHandle);
    if (SpecHandle.IsValid())
    {
        ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}

void UAugmentManagerComponent::OnRep_AppliedAugments()
{
    // Optionally update UI or effects when replicated
}

void UAugmentManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UAugmentManagerComponent, AppliedAugments);
}

