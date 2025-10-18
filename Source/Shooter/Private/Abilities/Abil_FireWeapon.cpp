#include "Abilities/Abil_FireWeapon.h"
#include "Characters/ShooterCharacter.h"
#include "Firearms/ShooterFirearm.h"
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

	AShooterCharacter* ShooterChar = Cast<AShooterCharacter>(ActorInfo->AvatarActor.Get());
	if (!ShooterChar)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Get equipped firearm (your AShooterFirearm)
	AShooterFirearm* Firearm = Cast<AShooterFirearm>(ShooterChar->GetEquippedWeapon());

	UE_LOG(LogTemp, Warning, TEXT("[FireAbility] Firearm=%s"), *GetNameSafe(Firearm));

	if (!Firearm)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveFirearm = Firearm;
	HandleFire(ShooterChar);
}

void UAbil_FireWeapon::HandleFire(AShooterCharacter* ShooterChar)
{
	if (!ActiveFirearm)
		return;

	// Local prediction call (client + server)
	ActiveFirearm->Fire();

	// If the firearm is full-auto, BeginAutoIfNeeded() runs inside Fire()
	// Burst/auto behavior already handled internally.
}

void UAbil_FireWeapon::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActiveFirearm)
	{
		ActiveFirearm->StopFire();
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
	if (ActiveFirearm)
	{
		ActiveFirearm->StopFire();
		ActiveFirearm = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
