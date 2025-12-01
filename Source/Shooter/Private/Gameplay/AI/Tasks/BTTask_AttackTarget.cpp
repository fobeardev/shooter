#include "Gameplay/AI/Tasks/BTTask_AttackTarget.h"
#include "Gameplay/Characters/AI/ShooterAICharacter.h"
#include "Gameplay/Tags/ShooterGameplayTags.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"

UBTTask_AttackTarget::UBTTask_AttackTarget()
{
    NodeName = TEXT("Attack Target");
}

EBTNodeResult::Type UBTTask_AttackTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    AShooterAICharacter* AIChar = Cast<AShooterAICharacter>(AIC ? AIC->GetPawn() : nullptr);
    if (!AIChar)
    {
        return EBTNodeResult::Failed;
    }

    if (UAbilitySystemComponent* ASC = AIChar->GetAbilitySystemComponent())
    {
        ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Fire"))));
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}
