#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ProjectileIdentity.generated.h"

USTRUCT(BlueprintType)
struct FProjectileIdentity
{
	GENERATED_BODY()

	// --- Identity ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ProjectileType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Element;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Pattern;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer Modifiers;

	// --- Ownership ---
	UPROPERTY()
	TWeakObjectPtr<class UAbilitySystemComponent> SourceASC;

	UPROPERTY()
	TWeakObjectPtr<AActor> InstigatorActor;
};
