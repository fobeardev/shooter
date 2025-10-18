#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Weapons/ShooterWeaponBase.h"
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

/**
 * Firearm implementation using SKG’s modular components.
 * - No dependency on ASKGFirearm (BP convenience class)
 * - Keeps your original component layout & BeginPlay init order
 * - Implements firing via SKG component API (ShotPerformed + Muzzle)
 */
UCLASS()
class SHOOTER_API AShooterFirearm : public AShooterWeaponBase
{
	GENERATED_BODY()

public:
	AShooterFirearm();

	// === Ability/Input entry points ===
	virtual void Fire() override;

	virtual void Server_Fire_Implementation() override;

	virtual void StopFire() override;

	// WeaponBase contracts
	virtual USkeletalMeshComponent* GetWeaponMesh() const override;
	virtual bool CanPerformAction() const override;

protected:
	// --- AActor
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|GAS")
	TSoftClassPtr<UGameplayEffect> DamageGameplayEffectClass;

	// --- Terminal Ballistics Configuration ---
	/** Bullet Data Asset defining mass, radius, drag, tracer, etc. */
	UPROPERTY(EditDefaultsOnly, Category = "Ballistics")
	TSoftObjectPtr<UBulletDataAsset> BulletDataAsset;

	// ------------------------------
	// SKG Components (keep these exact)
	// ------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Firearm")
	TObjectPtr<USkeletalMeshComponent> FirearmMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Firearm")
	TObjectPtr<USKGFirearmComponent> FirearmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Firearm")
	TObjectPtr<USKGAttachmentManagerComponent> AttachmentManagerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Firearm")
	TObjectPtr<USKGProceduralAnimComponent> ProceduralAnimComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Firearm")
	TObjectPtr<USKGMuzzleComponent> MuzzleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Firearm")
	TObjectPtr<USKGOffhandIKComponent> OffhandIKComponent;

protected:
	// ------------------------------
	// Firing Logic
	// ------------------------------
	virtual void HandleFire_Internal() override;
	virtual void HandleStopFire_Internal() override;

	void BeginAutoIfNeeded();     // set timer if Auto/Burst
	void ClearFireTimers();

	// Cosmetic hooks (Blueprint can implement)
	UFUNCTION(BlueprintImplementableEvent, Category = "Shooter|FX")
	void PlayFireEffects();

	virtual void LaunchProjectile(const FSKGMuzzleTransform& LaunchTransform, bool bIsLocalFire);

	// Server authoritative spawn entry (if you want a pure C++ path)
	UFUNCTION(Server, Reliable)
	void Server_LaunchProjectile(const FSKGMuzzleTransform& LaunchTransform);

	// --- TB dynamic delegate handlers ---
	UFUNCTION()
	void OnBulletHit_TB(const FTBImpactParams& Impact);

	UFUNCTION()
	void OnBulletUpdate_TB(const FTBProjectileFlightData& Flight);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Firearm|Projectile")
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Firearm|Projectile")
	float ProjectileVelocity = 30000.0f; // cm/s, about 300 m/s

	// Timers for Auto/Burst
	FTimerHandle AutoTimerHandle;

	// Optional: local “input edge” counters if you want parity with _Old
	int32 PressCount = 0;
	int32 ReleaseCount = 0;

	// Rate-of-fire fallback (if no data asset yet)
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Firearm")
	float FireRateSeconds = 0.12f;

	// Burst tracking
	int32 PendingBurstShots = 0;
	int32 BurstSize = 3;  // you can drive this from data later
};
