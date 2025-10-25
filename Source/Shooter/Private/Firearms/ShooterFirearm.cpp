#include "Firearms/ShooterFirearm.h"

// SKG
#include "Components/SKGFirearmComponent.h"
#include "Components/SKGAttachmentManagerComponent.h"
#include "Components/SKGProceduralAnimComponent.h"
#include "Components/SKGOffhandIKComponent.h"
#include "Components/SKGMuzzleComponent.h"
#include "Components/SKGShooterPawnComponent.h"

// TB
#include "Subsystems/TerminalBallisticsSubsystem.h"
#include "Core/TBBulletDataAsset.h"
#include "Core/TBStatics.h"
#include "Types/TBLaunchTypes.h"
#include "Types/TBProjectileId.h"
#include "Types/TBProjectileFlightData.h"
#include "Types/TBImpactParams.h"

// UE
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

// Abilities
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

// Tags
#include "Tags/ShooterGameplayTags.h"
#include <AbilitySystemGlobals.h>
#include <Characters/ShooterCombatCharacter.h>

AShooterFirearm::AShooterFirearm()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	// --- Components (unchanged order/roles) ---
	FirearmComponent = CreateDefaultSubobject<USKGFirearmComponent>(TEXT("FirearmComponent"));
	AttachmentManagerComponent = CreateDefaultSubobject<USKGAttachmentManagerComponent>(TEXT("AttachmentManagerComponent"));
	MuzzleComponent = CreateDefaultSubobject<USKGMuzzleComponent>(TEXT("MuzzleComponent"));
	OffhandIKComponent = CreateDefaultSubobject<USKGOffhandIKComponent>(TEXT("OffhandIKComponent"));

	// Default tags for this firearm (can be overridden in BP)
	WeaponClassTag = ShooterTags::Weapon_Class_AR;
	DamageTypeTag = ShooterTags::Damage_Ballistic;
	CurrentFireModeTag = ShooterTags::Weapon_FireMode_Semi;

	WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMeshComponent;

	if (WeaponMeshComponent)
	{
		WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMeshComponent->SetGenerateOverlapEvents(false);
		WeaponMeshComponent->SetIsReplicated(true);
	}
}

void AShooterFirearm::BeginPlay()
{
	Super::BeginPlay();

	if (ensure(FirearmComponent))
	{
		// Make sure the names are correct (safety if someone renames components)
		FirearmComponent->SetFirearmMeshComponentName(WeaponMeshComponent->GetFName());
		FirearmComponent->SetAttachmentManagerComponentName(
			AttachmentManagerComponent ? AttachmentManagerComponent->GetFName() : NAME_None);
		FirearmComponent->InitializeFirearmComponent();
	}

	if (WeaponMeshComponent)
	{
		WeaponMeshComponent->SetComponentTickEnabled(false);
	}

	if (ProceduralAnimComponent)
	{
		ProceduralAnimComponent->SetProceduralMeshName(WeaponMeshComponent->GetFName());
		ProceduralAnimComponent->InitializeProceduralAnimComponent();
	}
}

void AShooterFirearm::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Base already replicates CurrentFireModeTag & bIsReloading
}

// ===== WeaponBase contracts =====

bool AShooterFirearm::CanPerformAction() const
{
	// Enforce base rules + any firearm-specific ones here (e.g., not aiming transitions, chamber checks)
	return Super::CanPerformAction();
}

// ===== Firing =====

void AShooterFirearm::Fire()
{
	if (!CanPerformAction())
	{
		return;
	}

	// Local edge count for parity with your old graph (optional)
	++PressCount;

	// Client asks server to fire; server will then execute HandleFire_Internal (authoritative)
	Server_Fire();

	// Local prediction: if you want immediate client feedback, you can also call HandleFire_Internal() here
	// HandleFire_Internal();
}

void AShooterFirearm::Server_Fire_Implementation()
{
	HandleFire_Internal();
	BeginAutoIfNeeded();
}

void AShooterFirearm::StopFire()
{
	ClearFireTimers();
	++ReleaseCount;
}

// One shot worth of work
void AShooterFirearm::HandleFire_Internal()
{
	UE_LOG(LogTemp, Warning, TEXT("[Firearm] HandleFire_Internal called on %s"), *GetNameSafe(this));

	if (!FirearmComponent)
	{
		return;
	}

	// Ask SKG for the correct muzzle transform (handles current muzzle, offsets, zeroing, etc.)
	const FSKGMuzzleTransform MuzzleXform = FirearmComponent->GetMuzzleProjectileTransform(/*ZeroDistance*/100.f, /*MOA*/1.f);

	// Server spawns “real” projectile; clients can run cosmetic only
	if (HasAuthority())
	{
		Server_LaunchProjectile(MuzzleXform);
	}

	// SKG bookkeeping (heat, etc.)
	FirearmComponent->ShotPerformed();

	// Cosmetic local feedback
	PlayFireEffects();

	// (Optional) If you have a ShooterPawnComp cached, you can call:
	if (ShooterPawn) 
	{ 
		ShooterPawn->PerformProceduralRecoil(FRotator(1.f), FVector(1.f), FRotator(1.f)); 
	}
}

void AShooterFirearm::HandleStopFire_Internal()
{
	// Stop full-auto repeating timers (if you use one)
	if (GetWorldTimerManager().IsTimerActive(AutoTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(AutoTimerHandle);
	}
}

void AShooterFirearm::BeginAutoIfNeeded()
{
	if (CurrentFireModeTag == ShooterTags::Weapon_FireMode_Auto)
	{
		// Loop while held; a real implementation may read ROF from your data asset
		if (!GetWorldTimerManager().IsTimerActive(AutoTimerHandle))
		{
			GetWorldTimerManager().SetTimer(
				AutoTimerHandle,
				this,
				&AShooterFirearm::HandleFire_Internal,
				FMath::Max(0.01f, FireRateSeconds),
				true
			);
		}
	}
	else if (CurrentFireModeTag == ShooterTags::Weapon_FireMode_Burst)
	{
		PendingBurstShots = BurstSize - 1; // already fired one in Server_Fire
		if (PendingBurstShots > 0)
		{
			GetWorldTimerManager().SetTimer(
				AutoTimerHandle,
				[this]()
				{
					if (PendingBurstShots <= 0)
					{
						ClearFireTimers();
						return;
					}
					HandleFire_Internal();
					--PendingBurstShots;
				},
				FMath::Max(0.01f, FireRateSeconds),
				true
			);
		}
	}
}

void AShooterFirearm::ClearFireTimers()
{
	GetWorldTimerManager().ClearTimer(AutoTimerHandle);
	PendingBurstShots = 0;
}

void AShooterFirearm::LaunchProjectile(const FSKGMuzzleTransform& LaunchTransform, bool bIsLocalFire)
{
	if (!GetWorld())
		return;

	const FTransform Xf = LaunchTransform.ConvertToTransform();
	const FVector FireLoc = Xf.GetLocation();
	const FVector FireDir = Xf.GetRotation().GetForwardVector();

	// Build Launch Params (note: Owner is set inside helper; required by TB statics)
	FTBLaunchParams LaunchParams = UTerminalBallisticsStatics::MakeLaunchParamsWithDirectionVector(
		/*ProjectileSpeed m/s*/        900.0,
		/*EffectiveRange m*/           5000.0,
		/*Timescale*/                  1.0,
		/*Location*/                   FireLoc,
		/*Direction*/                  FireDir,
		/*ActorsToIgnore*/{ this },
		/*ObjTypes*/                   UTerminalBallisticsStatics::GetDefaultObjectTypes(),
		/*TraceChannel*/               ECC_GameTraceChannel10,
		/*bIgnoreOwner*/               true,
		/*bAddToOwnerVelocity*/        true,
		/*bForceNoTracer*/             false,
		/*Owner*/                      this,
		/*InstigatorController*/       GetInstigatorController(),
		/*GravityMultiplier*/          1.0,
		/*OwnerIgnoreDistance*/        10.0,
		/*TracerActivationDistance*/   25.0,
		/*Payload*/                    nullptr
	);

	UBulletDataAsset* Bullet = nullptr;

	if (BulletDataAsset.IsValid())
	{
		Bullet = BulletDataAsset.Get();
	}
	else if (BulletDataAsset.ToSoftObjectPath().IsValid())
	{
		Bullet = BulletDataAsset.LoadSynchronous();
	}

	if (!Bullet)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShooterFirearm] No BulletDataAsset set on %s"), *GetNameSafe(this));
		return;
	}

	const int32 DebugFlags =
		(int32)ETBBallisticsDebugType::DrawDebugTrace |
		(int32)ETBBallisticsDebugType::PrintDebugInfo;

	// Dynamic delegates (Blueprint-style) — bind to UFUNCTIONs on this object
	FBPOnBulletHit OnHitBP;
	OnHitBP.BindUFunction(this, FName("OnBulletHit_TB"));

	FBPOnProjectileUpdate OnUpdateBP;
	OnUpdateBP.BindUFunction(this, FName("OnBulletUpdate_TB"));

	// Empty delegates we don't currently need
	FBPOnProjectileComplete OnCompleteBP;
	FBPOnBulletExitHit OnExitHitBP;
	FBPOnBulletInjure OnInjureBP;

	// Fire the bullet
	UTerminalBallisticsStatics::AddAndFireBulletWithCallbacksAndUpdate(
		Bullet,
		LaunchParams,
		OnCompleteBP,
		OnHitBP,
		OnExitHitBP,
		OnInjureBP,
		OnUpdateBP,
		FTBProjectileId::None,
		DebugFlags
	);
}

// ===== Projectile spawn server path =====

void AShooterFirearm::Server_LaunchProjectile_Implementation(const FSKGMuzzleTransform& LaunchTransform)
{
	// Only run on server — spawns the authoritative projectile
	LaunchProjectile(LaunchTransform, /*bIsLocalFire=*/false);
}

// --- TB delegate: OnHit (apply GAS damage) ---
void AShooterFirearm::OnBulletHit_TB(const FTBImpactParams& Impact)
{
	// Server-authoritative damage
	if (!HasAuthority())
		return;


	AActor* HitActor = Impact.HitResult.GetActor();
	if (!HitActor || HitActor == this)
		return;

	if (AShooterCombatCharacter* TargetChar = Cast<AShooterCombatCharacter>(HitActor))
	{
		if (TargetChar->IsDead()) 
		{
			UE_LOG(LogTemp, Warning, TEXT("[ShooterFirearm] TargetChar is already dead, from: %s"), *GetNameSafe(this));
			return; // don't apply damage to dead bodies
		}
	}

	// Source ASC (weapon owner) -> Target ASC (hit actor)
	UAbilitySystemComponent* SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor);
	if (!SourceASC || !TargetASC)
		return;
	if (!SourceASC || !TargetASC)
		return;

	// --- Compute basic ballistic energy scalar ---
	UBulletDataAsset* Bullet = nullptr;

	if (BulletDataAsset.IsValid())
	{
		Bullet = BulletDataAsset.Get();
	}
	else if (BulletDataAsset.ToSoftObjectPath().IsValid())
	{
		Bullet = BulletDataAsset.LoadSynchronous();
	}

	if (!Bullet)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShooterFirearm] No BulletDataAsset set on %s"), *GetNameSafe(this));
		return;
	}

	const float ImpactSpeed = Impact.ImpactVelocity.Size();

	// Approximate "gamey" kinetic energy model
	const float MassKg = Bullet->BulletProperties.Mass; // 4 g 5.56 NATO
	const float VelocityMS = Impact.ImpactVelocity.Size() / 100.f;
	float KE = 0.5f * MassKg * FMath::Square(VelocityMS);

	// Map real kinetic energy -> gameplay damage range
	float ImpactEnergy = FMath::GetMappedRangeValueClamped(
		FVector2D(0.f, 3500.f),   // realistic pistol -> rifle range in joules
		FVector2D(10.f, 50.f),    // gameplay damage window
		KE
	);

	// --- Load ballistic damage GE from soft reference ---
	TSubclassOf<UGameplayEffect> DamageGE = DamageGameplayEffectClass.LoadSynchronous();
	if (!DamageGE)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TB] Missing DamageGameplayEffectClass on %s"), *GetName());
		return;
	}

	// --- Prepare context and spec ---
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddInstigator(GetOwner(), this);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageGE, 1.0f, Context);
	if (!SpecHandle.IsValid())
		return;

	// --- Pass dynamic damage value ---
	SpecHandle.Data->SetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag(TEXT("Data.Weapon.DamageScalar")),
		-ImpactEnergy
	);

	// --- Apply damage ---
	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	UE_LOG(LogTemp, Log, TEXT("[TB] Hit %s | Speed=%.1f | Energy=%.1f | Surface=%d"),
		*GetNameSafe(HitActor),
		ImpactSpeed,
		ImpactEnergy,
		(int32)Impact.SurfaceType);
}

// --- TB delegate: per-tick flight update (optional logging) ---
void AShooterFirearm::OnBulletUpdate_TB(const FTBProjectileFlightData& Flight)
{
	UE_LOG(LogTemp, Verbose, TEXT("[TB] Bullet @ %s | Vel=%.1f"),
		*Flight.Location.ToString(),
		Flight.Velocity.Size());
}