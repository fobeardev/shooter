// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FlankMove.generated.h"

/**
 * Moves the AI to a flanking position around the TargetActor (±90° offset),
 * keeping focus on the player while strafing sideways.
 */
UCLASS()
class SHOOTER_API UBTTask_FlankMove : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FlankMove();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, Category = "Flank")
	float FlankDistance = 600.f;

	UPROPERTY(EditAnywhere, Category = "Flank")
	FName TargetActorKey = "TargetActor";

	UPROPERTY(EditAnywhere, Category = "Flank")
	FName IsFlankingKey = "bIsFlanking";

	bool bMoving = false;
};
