// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Animation/AnimMontage.h"
#include "BTTask_PlayMontage.generated.h"

/**
 * Plays a specified AnimMontage on the controlled pawn’s mesh.
 * Returns Success when montage completes, or Fails if it can’t play.
 */
UCLASS()
class SHOOTER_API UBTTask_PlayMontage : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_PlayMontage();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
	UPROPERTY(EditAnywhere, Category = "Montage")
	TObjectPtr<UAnimMontage> MontageToPlay;

	UPROPERTY(EditAnywhere, Category = "Montage")
	bool bLoopUntilInterrupted = false;

	UPROPERTY()
	float MontageLength = 0.f;

	UPROPERTY()
	float TimeElapsed = 0.f;
};
