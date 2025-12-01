// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/AI/ShooterAICharacter.h"
#include "ShooterAI_Marksman.generated.h"

/**
 * Ranged archetype: maintains distance and fires periodically.
 */
UCLASS()
class SHOOTER_API AShooterAI_Marksman : public AShooterAICharacter
{
    GENERATED_BODY()

public:
    AShooterAI_Marksman();

protected:
    virtual void BeginPlay() override;

    // Preferred minimum distance to player
    UPROPERTY(EditDefaultsOnly, Category = "Marksman|Combat")
    float MinEngageDistance = 600.f;

    // Fire interval
    UPROPERTY(EditDefaultsOnly, Category = "Marksman|Combat")
    float FireInterval = 1.2f;

    FTimerHandle FireTimerHandle;
    void TryRangedAttack();
};
