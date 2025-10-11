#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/** Checks if target can currently be damaged (ignores during i-frames). */
bool SHOOTER_API CanApplyDamageTo(AActor* Target);
