#pragma once
#include "CoreMinimal.h"
#include "TriggerData.generated.h"

USTRUCT(BlueprintType)
struct FTriggerData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    int32 Pressed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    int32 Released = 0;
};
