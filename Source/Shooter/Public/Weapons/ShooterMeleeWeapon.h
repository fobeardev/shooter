// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/ShooterWeaponBase.h"
#include "Tags/ShooterGameplayTags.h"
#include "ShooterMeleeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterMeleeWeapon : public AShooterWeaponBase
{
    GENERATED_BODY()

public:

    AShooterMeleeWeapon();

protected:
    virtual void HandleFire_Internal() override;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float Damage = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float Range = 200.f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    UAnimMontage* AttackMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|FX")
    FGameplayTag HitCueTag = ShooterTags::GameplayCue_Combat_MeleeHit;
};
