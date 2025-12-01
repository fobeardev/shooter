#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "AugmentData.generated.h"

class UGameplayEffect;


USTRUCT(BlueprintType)
struct FAugmentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag AugmentTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UGameplayEffect> GameplayEffectClass;

    FAugmentData()
        : DisplayName(FText::FromString("Default Augment")),
        AugmentTag(),
        GameplayEffectClass(nullptr)
    {
    }
};
