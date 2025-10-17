#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Abil_FireWeapon.generated.h"

class AShooterCharacter;
class AShooterFirearm;

/**
 * GAS ability that handles weapon firing logic.
 *
 * Activation:
 *  - Bound to GameplayTag: Ability.Weapon.Fire
 *  - Activated by Input_FirePressed()
 *  - Ends when Input_FireReleased() or owner loses weapon.
 */
UCLASS()
class SHOOTER_API UAbil_FireWeapon : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAbil_FireWeapon();

	// Core overrides
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	/** Cached weapon reference for duration of activation */
	UPROPERTY()
	TObjectPtr<AShooterFirearm> ActiveFirearm = nullptr;

	/** Internal helper for firing logic */
	void HandleFire(AShooterCharacter* ShooterChar);
};
