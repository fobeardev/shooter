#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Combat/Weapons/Base/ShooterWeaponBase.h"
#include "Gameplay/Combat/Projectile/ProjectileConfig.h"
#include "Gameplay/Combat/Projectile/ProjectileIdentity.h"

#include "ShooterFirearm.generated.h"

class UGameplayEffect;
class UBulletDataAsset;

// SKG forward decls
class USKGFirearmComponent;
class USKGAttachmentManagerComponent;
class USKGProceduralAnimComponent;
class USKGOffhandIKComponent;
class USKGMuzzleComponent;
struct FSKGMuzzleTransform;

UCLASS()
class SHOOTER_API AShooterFirearm : public AShooterWeaponBase
{
    GENERATED_BODY()

public:
    AShooterFirearm();

    virtual void Fire() override;
    virtual void FireWithProjectileSpec(const FProjectileConfig& Config, const FProjectileIdentity& Identity) override;
    virtual void Server_FireWithProjectileSpec_Implementation(
        const FProjectileConfig& Config,
        const FProjectileIdentity& Identity
    ) override;

    virtual void StopFire() override;
    
    virtual bool CanPerformAction() const override;

    // Phase 2: ability assembles config; firearm stores and uses it during auto/burst timers.
    void SetFireConfig(const FProjectileConfig& InConfig);
    const FProjectileConfig& GetFireConfig() const { return FireConfig; }

    // Ability may need muzzle without firing.
    FSKGMuzzleTransform GetMuzzleTransform() const;

    // Ability reads these to build config.
    const TSoftObjectPtr<UBulletDataAsset>& GetBulletDataAsset() const { return BulletDataAsset; }
    const TSoftObjectPtr<UBulletDataAsset>& GetRicochetBulletDataAsset() const { return RicochetBulletDataAsset; }

    // Ability reads this to inject into config.
    const TSoftClassPtr<UGameplayEffect>& GetDamageGameplayEffectClass() const { return DamageGameplayEffectClass; }

    const FProjectileConfig& GetBaseProjectileConfig() const { return BaseProjectileConfig; }
    const FProjectileIdentity& GetBaseProjectileIdentity() const { return BaseProjectileIdentity; }

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|GAS")
    TSoftClassPtr<UGameplayEffect> DamageGameplayEffectClass;

    // Still stored on the firearm as weapon tuning data (Phase 2 reads these from ability).
    UPROPERTY(EditDefaultsOnly, Category = "Ballistics")
    TSoftObjectPtr<UBulletDataAsset> BulletDataAsset;

    UPROPERTY(EditDefaultsOnly, Category = "Ballistics")
    TSoftObjectPtr<UBulletDataAsset> RicochetBulletDataAsset;

    // SKG Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Firearm")
    TObjectPtr<USKGFirearmComponent> FirearmComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Firearm")
    TObjectPtr<USKGAttachmentManagerComponent> AttachmentManagerComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Firearm")
    TObjectPtr<USKGMuzzleComponent> MuzzleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Firearm")
    TObjectPtr<USKGOffhandIKComponent> OffhandIKComponent;
    
    // Designer-tunable base projectile (Phase 3 foundation)
    UPROPERTY(EditDefaultsOnly, Category = "Firearm|Projectile")
    FProjectileConfig BaseProjectileConfig;

    UPROPERTY(EditDefaultsOnly, Category = "Firearm|Projectile")
    FProjectileIdentity BaseProjectileIdentity;

    virtual void HandleFire_Internal() override;
    virtual void HandleStopFire_Internal() override;

    void BeginAutoIfNeeded();
    void ClearFireTimers();

    UFUNCTION(BlueprintImplementableEvent, Category = "Shooter|FX")
    void PlayFireEffects();

    // Phase 2: firearm only spawns using a config already assembled by ability.
    void LaunchProjectile(const FSKGMuzzleTransform& LaunchTransform, const FProjectileConfig& Config, bool bIsLocalFire);

protected:
    // Cached config used by auto/burst timer shots.
    FProjectileConfig FireConfig;

    // Timers for Auto/Burst
    FTimerHandle AutoTimerHandle;

    int32 PressCount = 0;
    int32 ReleaseCount = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Shooter|Firearm")
    float FireRateSeconds = 0.12f;

    int32 PendingBurstShots = 0;
    int32 BurstSize = 3;
};
