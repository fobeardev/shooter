// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/AI/ShooterAI_Marksman.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Tags/ShooterGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Gameplay/Combat/Weapons/Base/ShooterWeaponBase.h"

AShooterAI_Marksman::AShooterAI_Marksman()
{
    ArchetypeTag = ShooterTags::Enemy_Type_Marksman;
}

void AShooterAI_Marksman::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority() && DefaultWeaponClass)
    {
        SpawnDefaultWeapon_Internal();
    }

    GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AShooterAI_Marksman::TryRangedAttack, FireInterval, true);
}

void AShooterAI_Marksman::TryRangedAttack()
{
    if (bIsDead || !ASC) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    const float Dist = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    if (Dist > MinEngageDistance)
    {
        // Use the same gameplay tag system as the player
        FGameplayTagContainer FireTags(ShooterTags::Ability_Weapon_Fire);
        ASC->TryActivateAbilitiesByTag(FireTags);
    }
}
