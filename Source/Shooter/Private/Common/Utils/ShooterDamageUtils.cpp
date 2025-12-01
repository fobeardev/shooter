#include "Common/Utils/ShooterDamageUtils.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"

bool CanApplyDamageTo(AActor* Target)
{
	if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target))
	{
		static const FGameplayTag Tag_IFrame = FGameplayTag::RequestGameplayTag(TEXT("State.IFrame"));
		if (TargetASC->HasMatchingGameplayTag(Tag_IFrame))
		{
			return false; // in i-frames, ignore hit
		}
	}
	return true;
}
