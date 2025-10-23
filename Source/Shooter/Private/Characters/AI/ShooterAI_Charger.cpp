// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/AI/ShooterAI_Charger.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Tags/ShooterGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Weapons/ShooterWeaponBase.h"

AShooterAI_Charger::AShooterAI_Charger()
{
    // Identify archetype
    ArchetypeTag = ShooterTags::Enemy_Type_Charger;
}

void AShooterAI_Charger::BeginPlay()
{
    Super::BeginPlay();

    // Start repeating melee checks
    GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AShooterAI_Charger::PerformMeleeAttack, AttackInterval, true);
}

void AShooterAI_Charger::PerformMeleeAttack()
{
    if (bIsDead || !ASC) return;

    FGameplayTagContainer MeleeTags(ShooterTags::Ability_Weapon_Fire);
    ASC->TryActivateAbilitiesByTag(MeleeTags);
}
