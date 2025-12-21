#pragma once

// Engine headers
#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"

// Project headers
#include "Gameplay/Combat/Weapons/Firearms/ShooterFirearm.h"
#include "Gameplay/Combat/Projectile/ProjectileConfig.h"
#include "Gameplay/Combat/Projectile/ProjectileIdentity.h"
#include "Gameplay/Tags/ShooterGameplayTags.h"

#include "Abil_FireWeapon.generated.h"

class AShooterWeaponBase;
class AShooterFirearm;
class UBulletDataAsset;
class UAbilitySystemComponent;

UCLASS()
class SHOOTER_API UAbil_FireWeapon : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UAbil_FireWeapon();

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

private:
    UBulletDataAsset* ResolveBulletDataAsset(AShooterFirearm* Firearm, UAbilitySystemComponent* ASC) const;
    FProjectileConfig BuildProjectileConfig(AShooterFirearm* Firearm, UBulletDataAsset* Bullet) const;

private:
    UPROPERTY()
    TObjectPtr<AShooterWeaponBase> ActiveWeapon;
};
