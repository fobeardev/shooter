// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/ShooterCombatCharacter.h"
#include "ShooterAICharacter.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterAICharacter : public AShooterCombatCharacter
{
    GENERATED_BODY()

public:
	AShooterAICharacter();

	/** Tag identifying the AI archetype (e.g. Enemy.Type.Charger / Enemy.Type.Marksman). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	FGameplayTag ArchetypeTag;
protected:
    virtual void BeginPlay() override;
};
