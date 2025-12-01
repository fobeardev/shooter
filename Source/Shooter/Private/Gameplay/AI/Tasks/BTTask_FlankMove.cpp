// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/AI/Tasks/BTTask_FlankMove.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/KismetMathLibrary.h"

UBTTask_FlankMove::UBTTask_FlankMove()
{
	NodeName = TEXT("Move To Flank Position");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_FlankMove::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    constexpr bool bStopOnOverlap = true;
    constexpr bool bUsePathfinding = true;
    constexpr bool bProjectToNav = false;
    constexpr bool bCanStrafe = true;
    constexpr bool bAllowPartial = true;

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

    // Determine direction from target -> AI
    FVector ToTarget = (AIPawn->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal2D();

    // ±90° flank rotation
    const float Side = FMath::RandBool() ? 1.f : -1.f;
    FVector FlankDir = ToTarget.RotateAngleAxis(Side * 90.f, FVector::UpVector);
    FVector DesiredLoc = Target->GetActorLocation() + (FlankDir * FlankDistance);

    // Project to navmesh
    FNavLocation NavLoc;
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys || !NavSys->ProjectPointToNavigation(DesiredLoc, NavLoc))
        return EBTNodeResult::Failed;

    // Tell AI to face the target (this mimics "Allow Strafe")
    Controller->SetFocus(Target);

    // Move to the flank position
    const EPathFollowingRequestResult::Type Result = Controller->MoveToLocation(NavLoc.Location, 50.f, bStopOnOverlap, bUsePathfinding,
        bProjectToNav, bCanStrafe, nullptr, bAllowPartial);

    if (Result == EPathFollowingRequestResult::RequestSuccessful)
    {
        bMoving = true;
        return EBTNodeResult::InProgress;
    }

    return EBTNodeResult::Failed;
}

void UBTTask_FlankMove::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	EPathFollowingStatus::Type MoveStatus = Controller->GetMoveStatus();

	if (MoveStatus == EPathFollowingStatus::Idle)
	{
		// Stop looking at target when done flanking
		Controller->ClearFocus(EAIFocusPriority::Gameplay);

		// Clear blackboard flanking flag
		if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
		{
			BB->SetValueAsBool(IsFlankingKey, false);
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

