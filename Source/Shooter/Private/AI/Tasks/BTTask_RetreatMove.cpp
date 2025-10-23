// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_RetreatMove.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_RetreatMove::UBTTask_RetreatMove()
{
	NodeName = TEXT("Retreat Away From Target");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_RetreatMove::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller)
		return EBTNodeResult::Failed;

	APawn* AIPawn = Controller->GetPawn();
	if (!AIPawn)
		return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey));
	if (!Target)
		return EBTNodeResult::Failed;

	// Compute "away from target" vector
	FVector DirAway = (AIPawn->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal2D();
	FVector RetreatLocation = AIPawn->GetActorLocation() + (DirAway * RetreatDistance);

	// Ensure it's on the navmesh
	FNavLocation NavLoc;
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys && NavSys->ProjectPointToNavigation(RetreatLocation, NavLoc))
	{
		Controller->MoveToLocation(NavLoc.Location);
		bMoving = true;
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_RetreatMove::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (Controller->GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
