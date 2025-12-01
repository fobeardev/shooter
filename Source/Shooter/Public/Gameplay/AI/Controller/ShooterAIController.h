#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ShooterAIController.generated.h"

/**
 * Simple combat AI controller.
 * Immediately acquires the player and begins combat without perception checks.
 */
UCLASS()
class SHOOTER_API AShooterAIController : public AAIController
{
    GENERATED_BODY()

public:
    AShooterAIController();

protected:
    virtual void BeginPlay() override;

    // Optional configurable delay before first attack (0.5–1.5s randomized)
    UPROPERTY(EditDefaultsOnly, Category = "Shooter|AI")
    bool bUseRandomizedAttackDelay = true;

    UPROPERTY(EditDefaultsOnly, Category = "Shooter|AI")
    float MinAttackDelay = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Shooter|AI")
    float MaxAttackDelay = 1.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Shooter|AI")
    TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

private:
    void InitializeCombatState();
};
