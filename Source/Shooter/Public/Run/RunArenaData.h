// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "RunArenaData.generated.h"

UCLASS(BlueprintType)
class SHOOTER_API URunArenaData : public UDataAsset
{
    GENERATED_BODY()

public:
    // Arena map to load (soft reference for async streaming)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UWorld> ArenaLevel;

    // Difficulty tier or scaling factor
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 DifficultyTier = 1;

    // Biome identifier for aesthetics or enemy sets
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag BiomeTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bIsFinalArena = false;
};
