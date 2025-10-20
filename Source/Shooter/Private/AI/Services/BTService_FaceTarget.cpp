// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Services/BTService_FaceTarget.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_FaceTarget::UBTService_FaceTarget()
{
    NodeName = TEXT("Face Target");
    bNotifyBecomeRelevant = true;
    bNotifyCeaseRelevant = false;
    Interval = 0.1f; // tick every 0.1s
}

void UBTService_FaceTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC) return;

    APawn* Pawn = AIC->GetPawn();
    if (!Pawn) return;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(GetSelectedBlackboardKey()));
    if (!Target) return;

    FVector Direction = (Target->GetActorLocation() - Pawn->GetActorLocation());
    Direction.Z = 0.f;

    if (!Direction.IsNearlyZero())
    {
        FRotator NewRot = Direction.Rotation();
        Pawn->SetActorRotation(NewRot);
    }
}
