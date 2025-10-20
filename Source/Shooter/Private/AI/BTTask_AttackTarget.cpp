#include "AI/BTTask_AttackTarget.h"
#include "AIController.h"
#include "Characters/ShooterAICharacter.h"
#include "AbilitySystemComponent.h"
#include "Tags/ShooterGameplayTags.h"

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
