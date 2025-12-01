// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/AI/Tasks/BTTask_PlayMontage.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

UBTTask_PlayMontage::UBTTask_PlayMontage()
{
	NodeName = TEXT("Play Montage");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_PlayMontage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller)
		return EBTNodeResult::Failed;

	ACharacter* Char = Cast<ACharacter>(Controller->GetPawn());
	if (!Char || !MontageToPlay)
		return EBTNodeResult::Failed;

	UAnimInstance* AnimInstance = Char->GetMesh() ? Char->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
		return EBTNodeResult::Failed;

	// Play the montage
	MontageLength = AnimInstance->Montage_Play(MontageToPlay, 1.0f);
	TimeElapsed = 0.f;

	if (MontageLength <= 0.f)
		return EBTNodeResult::Failed;

	// If looping, we’ll stay active until externally aborted
	if (bLoopUntilInterrupted)
	{
		return EBTNodeResult::InProgress;
	}

	// Otherwise tick until it’s done
	return EBTNodeResult::InProgress;
}

void UBTTask_PlayMontage::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	TimeElapsed += DeltaSeconds;

	if (!bLoopUntilInterrupted && TimeElapsed >= MontageLength)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

void UBTTask_PlayMontage::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	AAIController* Controller = OwnerComp.GetAIOwner();
	if (!Controller)
		return;

	ACharacter* Char = Cast<ACharacter>(Controller->GetPawn());
	if (Char && Char->GetMesh())
	{
		if (UAnimInstance* AnimInstance = Char->GetMesh()->GetAnimInstance())
		{
			if (MontageToPlay && AnimInstance->Montage_IsPlaying(MontageToPlay))
			{
				AnimInstance->Montage_Stop(0.25f, MontageToPlay);
			}
		}
	}
}
