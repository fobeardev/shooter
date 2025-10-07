#include "Abilities/AttrSet_Combat.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UAttrSet_Combat::UAttrSet_Combat()
{
	// MVP defaults
	InitMaxHealth(100.f);
	InitHealth(100.f);
	InitMoveSpeed(600.f);
}

void UAttrSet_Combat::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttrSet_Combat, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttrSet_Combat, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAttrSet_Combat, MoveSpeed, COND_None, REPNOTIFY_Always);
}

void UAttrSet_Combat::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAttrSet_Combat, Health, OldValue);
}

void UAttrSet_Combat::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAttrSet_Combat, MaxHealth, OldValue);
}

void UAttrSet_Combat::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAttrSet_Combat, MoveSpeed, OldValue);
}

void UAttrSet_Combat::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Clamp health to [0, MaxHealth]
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		const float NewHealth = FMath::Clamp(GetHealth(), 0.f, GetMaxHealth());
		SetHealth(NewHealth);
		// If you want to trigger death here when NewHealth <= 0, you can do it later.
	}
}
