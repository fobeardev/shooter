// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_FaceTarget.generated.h"

/**
 * Forces the AI pawn to face the current TargetActor each tick.
 */
UCLASS()
class SHOOTER_API UBTService_FaceTarget : public UBTService_BlackboardBase
{
    GENERATED_BODY()

public:
    UBTService_FaceTarget();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
