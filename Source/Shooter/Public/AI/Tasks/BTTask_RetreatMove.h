// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RetreatMove.generated.h"

UCLASS()
class SHOOTER_API UBTTask_RetreatMove : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_RetreatMove();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY(EditAnywhere, Category = "Retreat")
	float RetreatDistance = 400.f;

	UPROPERTY(EditAnywhere, Category = "Retreat")
	FName TargetActorKey = "TargetActor";

	bool bMoving = false;
};
