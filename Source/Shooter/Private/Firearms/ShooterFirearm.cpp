#include "Firearms/ShooterFirearm.h"

// SKG
#include "Components/SKGFirearmComponent.h"
#include "Components/SKGAttachmentManagerComponent.h"
#include "Components/SKGProceduralAnimComponent.h"
#include "Components/SKGOffhandIKComponent.h"
#include "Components/SKGMuzzleComponent.h"
#include "Components/SKGShooterPawnComponent.h"

// TB
#include <Subsystems/TerminalBallisticsSubsystem.h>
#include <Core/TBStatics.h>
#include <Types/TBLaunchTypes.h>
#include <Types/TBProjectile.h>
#include <Types/TBEnums.h>

// UE
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

// Abilities
#include "AbilitySystemBlueprintLibrary.h"

// Tags
#include "Tags/ShooterGameplayTags.h"

AShooterFirearm::AShooterFirearm()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	// --- Components (unchanged order/roles) ---
	FirearmMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirearmMeshComponent"));
	RootComponent = FirearmMeshComponent;

	FirearmComponent = CreateDefaultSubobject<USKGFirearmComponent>(TEXT("FirearmComponent"));
	AttachmentManagerComponent = CreateDefaultSubobject<USKGAttachmentManagerComponent>(TEXT("AttachmentManagerComponent"));
	ProceduralAnimComponent = CreateDefaultSubobject<USKGProceduralAnimComponent>(TEXT("ProceduralAnimComponent"));
	MuzzleComponent = CreateDefaultSubobject<USKGMuzzleComponent>(TEXT("MuzzleComponent"));
	OffhandIKComponent = CreateDefaultSubobject<USKGOffhandIKComponent>(TEXT("OffhandIKComponent"));

	// SKG init style (manual name wiring; we’ll init in BeginPlay)
	if (FirearmComponent)
	{
		FirearmComponent->bAutoInitialize = false;
		FirearmComponent->SetFirearmMeshComponentName(FirearmMeshComponent->GetFName());
		FirearmComponent->SetAttachmentManagerComponentName(AttachmentManagerComponent->GetFName());
	}

	if (ProceduralAnimComponent)
	{
		ProceduralAnimComponent->bAutoInitialize = true;
		ProceduralAnimComponent->bOverrideComponentNames = false;
	}

	// Default tags for this firearm (can be overridden in BP)
	WeaponClassTag = ShooterTags::Weapon_Class_AR;
	DamageTypeTag = ShooterTags::Damage_Ballistic;
	CurrentFireModeTag = ShooterTags::Weapon_FireMode_Semi;
}

void AShooterFirearm::BeginPlay()
{
	Super::BeginPlay();

	// Resolve & initialize SKG internals
	if (ensure(FirearmComponent))
	{
		// Make sure the names are correct (safety if someone renames components)
		FirearmComponent->SetFirearmMeshComponentName(FirearmMeshComponent->GetFName());
		FirearmComponent->SetAttachmentManagerComponentName(
			AttachmentManagerComponent ? AttachmentManagerComponent->GetFName() : NAME_None);
		FirearmComponent->InitializeFirearmComponent();
	}

	if (FirearmMeshComponent)
	{
		FirearmMeshComponent->SetComponentTickEnabled(false);
	}
}

void AShooterFirearm::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Base already replicates CurrentFireModeTag & bIsReloading
}

// ===== WeaponBase contracts =====

USkeletalMeshComponent* AShooterFirearm::GetWeaponMesh() const
{
	return FirearmMeshComponent;
}

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

	// Cosmetic local feedback (FX + procedural recoil; Blueprint can wire recoil via ShooterPawnComp)
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

	const FTransform RealXform = LaunchTransform.ConvertToTransform();
	const FVector FireLoc = RealXform.GetLocation();
	const FVector FireDir = RealXform.GetRotation().GetForwardVector();

	FTBLaunchParams LaunchParams = UTerminalBallisticsStatics::MakeLaunchParamsWithDirectionVector(
		900.0,                 // ProjectileSpeed
		5000.0,                // EffectiveRange
		1.0,                   // Timescale
		FireLoc,               // Location
		FireDir,               // Direction
		{ this },              // ToIgnore
		UTerminalBallisticsStatics::GetDefaultObjectTypes(),
		ECC_GameTraceChannel10,// Trace channel
		true,                  // bIgnoreOwner
		true,                  // bAddToOwnerVelocity
		false,                 // bForceNoTracer
		this,                  // Owner
		GetInstigatorController(),
		1.0,                   // GravityMultiplier
		10.0,                  // OwnerIgnoreDistance
		25.0,                  // TracerActivationDistance
		nullptr                // Payload
	);

	// --- Simplified projectile & physics setup
	FTBProjectile Projectile;
	Projectile.Mass = 0.02;   // kg
	Projectile.Radius = 0.005;  // m

	FPhysMatProperties PhysProps;
	PhysProps.Density = 7850.0; // kg/m^3
	PhysProps.UltimateTensileStrength = 210.0;  // MPa (example)

	const int32 DebugFlags =
		(int32)ETBBallisticsDebugType::DrawDebugTrace |
		(int32)ETBBallisticsDebugType::PrintDebugInfo;

	FTBProjectileId FiredId =
		UTerminalBallisticsStatics::AddAndFireProjectile(
			Projectile,
			PhysProps,
			LaunchParams,
			FTBProjectileId::None,
			DebugFlags
		);

	UE_LOG(LogTemp, Log, TEXT("Server fired projectile with debug trace."));
}

// ===== Projectile spawn server path =====

void AShooterFirearm::Server_LaunchProjectile_Implementation(const FSKGMuzzleTransform& LaunchTransform)
{
	// Only run on server — spawns the authoritative projectile
	LaunchProjectile(LaunchTransform, /*bIsLocalFire=*/false);
}
