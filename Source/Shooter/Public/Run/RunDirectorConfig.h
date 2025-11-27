// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RunDirectorConfig.generated.h"

class URunArenaData;

/**
 * 
 */
UCLASS(BlueprintType)
class SHOOTER_API URunDirectorConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<URunArenaData*> ArenaSequence;
};