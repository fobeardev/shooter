#include "AI/ShooterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Characters/ShooterCharacter.h"
#include "Kismet/GameplayStatics.h"

AShooterAIController::AShooterAIController()
{
    bAttachToPawn = true;
}

void AShooterAIController::BeginPlay()
{
    Super::BeginPlay();

    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
    }

    InitializeCombatState();
}

void AShooterAIController::InitializeCombatState()
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShooterAIController: No Blackboard found."));
        return;
    }

    // Acquire player pawn immediately
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("ShooterAIController: No player pawn found."));
        return;
    }

    BB->SetValueAsObject(FName("TargetActor"), PlayerPawn);
    BB->SetValueAsBool(FName("bCombatActive"), true);

    // Randomized pacing delay
    float AttackDelay = bUseRandomizedAttackDelay
        ? FMath::FRandRange(MinAttackDelay, MaxAttackDelay)
        : 0.5f;

    UE_LOG(LogTemp, Warning, TEXT("ShooterAIController: Combat activated. Attack delay: %.2f"), AttackDelay);

    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(
        TimerHandle,
        [this, PlayerPawn]()
        {
            if (UBlackboardComponent* BBRef = GetBlackboardComponent())
            {
                // Ensure target still valid
                if (IsValid(PlayerPawn))
                {
                    BBRef->SetValueAsObject(FName("TargetActor"), PlayerPawn);
                    BBRef->SetValueAsBool(FName("bReadyToAttack"), true);
                }
            }
        },
        AttackDelay,
        false
    );
}
