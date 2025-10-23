#include "Abilities/Abil_FireWeapon.h"
#include "Characters/ShooterCharacter.h"
#include "Weapons/ShooterWeaponBase.h"
#include "AbilitySystemComponent.h"
#include "Tags/ShooterGameplayTags.h"

UAbil_FireWeapon::UAbil_FireWeapon()
{
	// Basic metadata
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Tag this ability as weapon fire
	SetAssetTags(FGameplayTagContainer(ShooterTags::Ability_Weapon_Fire));

	ActivationOwnedTags.AddTag(ShooterTags::Ability_Weapon_Fire);
}

void UAbil_FireWeapon::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Works for both player and AI
	AShooterCombatCharacter* CombatChar = Cast<AShooterCombatCharacter>(ActorInfo->AvatarActor.Get());
	if (!CombatChar)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AShooterWeaponBase* Weapon = CombatChar->GetEquippedWeapon();
	UE_LOG(LogTemp, Warning, TEXT("[FireAbility] Weapon=%s (Owner=%s)"), *GetNameSafe(Weapon), *GetNameSafe(CombatChar));

	if (!Weapon)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveWeapon = Weapon;
	ActiveWeapon->Fire();
}

void UAbil_FireWeapon::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActiveWeapon)
	{
		ActiveWeapon->StopFire();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UAbil_FireWeapon::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (ActiveWeapon)
	{
		ActiveWeapon->StopFire();
		ActiveWeapon = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
