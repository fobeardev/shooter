// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/AI/ShooterAICharacter.h"
#include "ShooterAI_Charger.generated.h"

/**
 * Melee archetype: charges at player and performs short-range slam attack.
 */
UCLASS()
class SHOOTER_API AShooterAI_Charger : public AShooterAICharacter
{
    GENERATED_BODY()

public:
    AShooterAI_Charger();

protected:
    virtual void BeginPlay() override;

    // Simple melee damage GE
    UPROPERTY(EditDefaultsOnly, Category = "Charger|Combat")
    TSubclassOf<class UGameplayEffect> MeleeDamageEffect;

    // Melee attack range
    UPROPERTY(EditDefaultsOnly, Category = "Charger|Combat")
    float AttackRange = 250.f;

    // Cooldown between melee slams
    UPROPERTY(EditDefaultsOnly, Category = "Charger|Combat")
    float AttackInterval = 1.0f;

    void PerformMeleeAttack();
    FTimerHandle AttackTimerHandle;
};
