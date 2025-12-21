#include "Gameplay/Abilities/Abil_FireWeapon.h"

#include "Gameplay/Characters/ShooterCombatCharacter.h"
#include "Gameplay/Combat/Weapons/Base/ShooterWeaponBase.h"
#include "Gameplay/Combat/Weapons/Firearms/ShooterFirearm.h"
#include "Gameplay/Tags/ShooterGameplayTags.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"

// TB bullet data asset (still used as tuning source in Phase 2)
#include "Core/TBBulletDataAsset.h"

UAbil_FireWeapon::UAbil_FireWeapon()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

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

    AShooterCombatCharacter* CombatChar = Cast<AShooterCombatCharacter>(ActorInfo->AvatarActor.Get());
    if (!CombatChar)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AShooterWeaponBase* Weapon = CombatChar->GetEquippedWeapon();
    if (!Weapon)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ActiveWeapon = Weapon;

    FProjectileConfig Config;
    FProjectileIdentity Identity;

    // If this is a firearm, use designer-authored base projectile spec
    if (AShooterFirearm* Firearm = Cast<AShooterFirearm>(Weapon))
    {
        Config = Firearm->GetBaseProjectileConfig();
        Identity = Firearm->GetBaseProjectileIdentity();
    }
    else
    {
        // Fallback defaults for non-firearm (keeps system robust)
        Config = FProjectileConfig();
        Identity = FProjectileIdentity();
    }

    // Populate ownership (critical for damage attribution)
    UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();
    Identity.SourceASC = SourceASC;
    Identity.InstigatorActor = ActorInfo->AvatarActor.Get();

    // Ensure identity always has sane defaults
    if (!Identity.ProjectileType.IsValid())
    {
        Identity.ProjectileType = ShooterTags::Projectile_Type_Bullet;
    }
    if (!Identity.Element.IsValid())
    {
        Identity.Element = ShooterTags::Projectile_Element_None;
    }
    if (!Identity.Pattern.IsValid())
    {
        Identity.Pattern = ShooterTags::Projectile_Pattern_Single;
    }

    // Apply augments -> projectile modifiers (server authoritative spawn will use these)
    if (SourceASC)
    {
        if (SourceASC->HasMatchingGameplayTag(ShooterTags::Augment_Projectile_Ricochet))
        {
            Identity.Modifiers.AddTag(ShooterTags::Projectile_Modifier_Ricochet);
            Config.MaxRicochets = FMath::Max(Config.MaxRicochets, 2);
        }

        if (SourceASC->HasMatchingGameplayTag(ShooterTags::Augment_Projectile_Pierce))
        {
            Identity.Modifiers.AddTag(ShooterTags::Projectile_Modifier_Pierce);
            Config.MaxPierces = FMath::Max(Config.MaxPierces, 2);
        }

        if (SourceASC->HasMatchingGameplayTag(ShooterTags::Augment_Projectile_Homing))
        {
            Identity.Modifiers.AddTag(ShooterTags::Projectile_Modifier_Homing);
            Config.HomingAcquireRadiusCm = FMath::Max(Config.HomingAcquireRadiusCm, 2500.0f);
            Config.HomingTurnRateDegPerSec = FMath::Max(Config.HomingTurnRateDegPerSec, 360.0f);
        }

        if (SourceASC->HasMatchingGameplayTag(ShooterTags::Augment_Projectile_Accelerate))
        {
            Identity.Modifiers.AddTag(ShooterTags::Projectile_Modifier_Accelerate);
            Config.AccelerationCmPerSec2 = FMath::Max(Config.AccelerationCmPerSec2, 12000.0f);
        }

        if (SourceASC->HasMatchingGameplayTag(ShooterTags::Augment_Projectile_SplitShot))
        {
            // Bullet-hell friendly
            Config.ProjectileCount = FMath::Max(Config.ProjectileCount, 3);
            Config.SpreadHalfAngleDeg = FMath::Max(Config.SpreadHalfAngleDeg, 4.5f);

            Identity.Pattern = ShooterTags::Projectile_Pattern_Spread;
            Identity.Modifiers.AddTag(ShooterTags::Projectile_Modifier_Accelerate); // optional synergy; remove if you dislike
        }
    }

    // Fire using the projectile spec
    ActiveWeapon->FireWithProjectileSpec(Config, Identity);
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

UBulletDataAsset* UAbil_FireWeapon::ResolveBulletDataAsset(AShooterFirearm* Firearm, UAbilitySystemComponent* ASC) const
{
    if (!Firearm || !ASC)
    {
        return nullptr;
    }

    // Ricochet augment selects a different bullet data asset
    if (ASC->HasMatchingGameplayTag(ShooterTags::Augment_Projectile_Ricochet))
    {
        const TSoftObjectPtr<UBulletDataAsset>& RicochetDA = Firearm->GetRicochetBulletDataAsset();

        if (RicochetDA.IsValid())
        {
            UE_LOG(LogTemp, Log, TEXT("[FireAbility] Using RicochetBulletDataAsset on %s"), *GetNameSafe(Firearm));
            return RicochetDA.Get();
        }

        if (RicochetDA.ToSoftObjectPath().IsValid())
        {
            UBulletDataAsset* Loaded = RicochetDA.LoadSynchronous();
            UE_LOG(LogTemp, Log, TEXT("[FireAbility] Loaded RicochetBulletDataAsset on %s"), *GetNameSafe(Firearm));
            return Loaded;
        }
    }

    // Default bullet asset
    const TSoftObjectPtr<UBulletDataAsset>& BulletDA = Firearm->GetBulletDataAsset();

    if (BulletDA.IsValid())
    {
        return BulletDA.Get();
    }

    if (BulletDA.ToSoftObjectPath().IsValid())
    {
        return BulletDA.LoadSynchronous();
    }

    return nullptr;
}

FProjectileConfig UAbil_FireWeapon::BuildProjectileConfig(AShooterFirearm* Firearm, UBulletDataAsset* Bullet) const
{
    FProjectileConfig Config;

    // Speed: TB is m/s, our projectile system uses cm/s
    Config.InitialSpeed = Bullet ? (Bullet->MuzzleVelocity * 100.0f) : 6000.0f;

    // Lifetime: keep bounded for bullet hell
    Config.MaxLifeTime = 3.0f;

    // Radius: simple mapping from TB enum
    if (Bullet)
    {
        if (Bullet->ProjectileSize == PS_Small)
        {
            Config.Radius = 1.5f;
        }
        else if (Bullet->ProjectileSize == PS_Medium)
        {
            Config.Radius = 2.5f;
        }
        else
        {
            Config.Radius = 4.0f;
        }
    }
    else
    {
        Config.Radius = 2.5f;
    }

    // Gravity: parity with your former TB launch (1.0 multiplier)
    Config.GravityScale = 1.0f;

    // Collision: keep same intent as your old TB trace channel usage
    Config.CollisionChannel = ECC_GameTraceChannel10;
    Config.bUseSphereTrace = true;

    // Multi projectile
    Config.ProjectileCount = Bullet ? FMath::Max(1, Bullet->ProjectileCount) : 1;

    // Spread: TB has spread params, but we keep Phase 2 safe and consistent:
    // if multiple projectiles, apply a small cone unless you tune it later.
    Config.SpreadHalfAngleDeg = (Config.ProjectileCount > 1) ? 2.5f : 0.0f;

    // FX: tracer system preferred, else default tracer
    Config.bSpawnTracerFX = true;
    if (Bullet)
    {
        if (Bullet->TracerSystem.ToSoftObjectPath().IsValid())
        {
            Config.TracerFX = Bullet->TracerSystem;
        }
        else
        {
            Config.TracerFX = Bullet->DefaultTracerSystem;
        }
    }

    // GAS damage payload (Phase 1 preserved damage model)
    if (Bullet)
    {
        Config.BulletMassKg = Bullet->BulletProperties.Mass;
    }

    if (Firearm)
    {
        Config.DamageGameplayEffectClass = Firearm->GetDamageGameplayEffectClass();
    }

    Config.SetByCallerDamageTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Weapon.DamageScalar"));

    // Fallback direct damage remains available if GE not set elsewhere
    Config.Damage = 20.0f;

    return Config;
}
