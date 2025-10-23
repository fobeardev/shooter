// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_CheckFlankOrRetreat.generated.h"

UCLASS()
class SHOOTER_API UBTService_CheckFlankOrRetreat : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_CheckFlankOrRetreat();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
