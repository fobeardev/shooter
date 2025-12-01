// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/AI/Services/BTService_CheckFlankOrRetreat.h"
#include "Gameplay/Characters/AI/ShooterAICharacter.h"
#include "Gameplay/Characters/ShooterCombatCharacter.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayCueManager.h"

UBTService_CheckFlankOrRetreat::UBTService_CheckFlankOrRetreat()
{
	NodeName = TEXT("Check Flank or Retreat");
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UBTService_CheckFlankOrRetreat::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller) return;

	AShooterAICharacter* Self = Cast<AShooterAICharacter>(Controller->GetPawn());
	if (!Self) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	const float Health = Self->GetHealth();
	const float MaxHealth = Self->GetMaxHealth();
	const float HealthPercent = (MaxHealth > 0.f) ? (Health / MaxHealth) : 1.f;

	// --- Retreat Logic
	if (HealthPercent < 0.4f && !BB->GetValueAsBool(TEXT("bIsRetreating")))
	{
		BB->SetValueAsBool(TEXT("bIsRetreating"), true);
		UGameplayCueManager::ExecuteGameplayCue_NonReplicated(Self, FGameplayTag::RequestGameplayTag(FName("GameplayCue.Enemy.Retreat")), FGameplayCueParameters());
		return;
	}

	// --- Flank Logic (skip if retreating)
	if (!BB->GetValueAsBool(TEXT("bIsRetreating")))
	{
		TArray<AActor*> Allies;
		UGameplayStatics::GetAllActorsOfClass(Self->GetWorld(), AShooterAICharacter::StaticClass(), Allies);

		for (AActor* Ally : Allies)
		{
			if (Ally != Self && FVector::Dist(Self->GetActorLocation(), Ally->GetActorLocation()) < 800.f)
			{
				BB->SetValueAsBool(TEXT("bIsFlanking"), true);
				UGameplayCueManager::ExecuteGameplayCue_NonReplicated(Self, FGameplayTag::RequestGameplayTag(FName("GameplayCue.Enemy.Flank")), FGameplayCueParameters());
				return;
			}
		}

		BB->SetValueAsBool(TEXT("bIsFlanking"), false);
	}
}
