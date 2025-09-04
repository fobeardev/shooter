// ShooterFirearm.cpp

#include "ShooterFirearm.h"
#include "Components/SKGShooterPawnComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "ShooterPDAFirearm.h"
#include "Components/SkeletalMeshComponent.h"

AShooterFirearm::AShooterFirearm()
{
	bReplicates = true;
	// ReSharper disable once CppVirtualFunctionCallInsideCtor
	SetReplicateMovement(true);
}

void AShooterFirearm::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterFirearm, ReloadCounter);
	DOREPLIFETIME(AShooterFirearm, bHasOwner);	
}

void AShooterFirearm::BeginPlay()
{
	Super::BeginPlay();

	// BP: StartingTransform = GetTransform()
	StartingTransform = GetTransform();

	// BP: FirearmMeshComponent->SetComponentTickEnabled(false)
	if (FirearmMeshComponent)
	{
		FirearmMeshComponent->SetComponentTickEnabled(false);
	}

	// Hook SKG “DAConstructionBundlesLoaded” (BP had an override event)
	// ASKGBaseActor exposes this multicast; name may be slightly different in your SKG.
	if (DAConstruction)
	{
		// If SKG exposes a dynamic delegate, bind to it; otherwise call once here is fine.
		OnDAConstructionBundlesLoaded();
	}
}

void AShooterFirearm::OnDAConstructionBundlesLoaded()
{
	// BP: Cast DAConstruction -> PDA_Firearm and use FireRate/FireModes/AnimBP
	if (const UShooterPDAFirearm* PDA = Cast<UShooterPDAFirearm>(DAConstruction))
	{
		SetFireRateDelay(PDA->FireRate);
		SetupFireModes(PDA->FireModes);

		if (PDA->AnimBP && FirearmMeshComponent)
		{
			FirearmMeshComponent->SetAnimInstanceClass(PDA->AnimBP);
		}
	}
}

void AShooterFirearm::SetOwningPawn(APawn* OwningPawn)
{
	// BP: SetOwner(OwningPawn)
	SetOwner(OwningPawn);

	// BP: CurrentShooterPawnComponent = GetShooterPawnComponent(OwningPawn)
	CurrentShooterPawnComponent = USKGShooterPawnComponent::GetShooterPawnComponent(OwningPawn);

	// BP: bHasOwner = IsValid(OwningPawn)
	bHasOwner = UKismetSystemLibrary::IsValid(OwningPawn);
	OnRep_HasOwner();
}

void AShooterFirearm::Interact(APawn* InteractingPawn)
{
	// BP: SetReplicateMovement(false)
	SetReplicateMovement(false);
}

void AShooterFirearm::Drop()
{
	// BP chain: SetOwner(None) → bHasOwner=false → Detach(KeepWorld,KeepWorld,KeepWorld)
	SetOwner(nullptr);
	bHasOwner = false;
	OnRep_HasOwner();

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// BP: K2_SetActorTransform(StartingTransform)
	FHitResult Dummy;
	K2_SetActorTransform(StartingTransform, false, Dummy, false);

	// BP: SetReplicateMovement(true)
	SetReplicateMovement(true);
}

void AShooterFirearm::SetupFireModes(const FGameplayTagContainer& FireModeTags)
{
	// BP: copy tags, set CurrentFireMode to first if available
	FireModes = FireModeTags;

	if (FireModes.Num() > 0)
	{
		CurrentFireMode = FireModes.GetByIndex(0);
	}
}

void AShooterFirearm::SetFireRateDelay(double FireRate)
{
	// In the BP it passes PDA.FireRate (shots/sec).  Delay is seconds between shots.
	FireRateDelay = (FireRate > 0.0) ? (1.0f / static_cast<float>(FireRate)) : 0.0f;
}

void AShooterFirearm::OnRep_HasOwner()
{
	// If you need UI/FX when ownership flips, do it here.
}

// ---- Shooting ----
void AShooterFirearm::PressTrigger()
{
	// BP: if (CanPerformAction) { Switch Has Authority → (Remote) call Server; (Authority) call Server too is fine }
	if (!CanPerformAction())
	{
		return;
	}

	// Call the server to mutate state (even if we are authority, it's harmless and keeps one path)
	Server_PressTrigger();
}

void AShooterFirearm::ReleaseTrigger()
{
	// BP releases compares TriggerData.Pressed > TriggerData.Released before doing work.
	// We'll do the same gate here (Reuse CanPerformAction for symmetry, or make a Release-specific check)
	if (TriggerData.Pressed <= TriggerData.Released)
	{
		return;
	}

	Server_ReleaseTrigger();
}

void AShooterFirearm::Server_PressTrigger_Implementation()
{
	IncreasePressTrigger();

	// You likely want to start continuous fire if CurrentFireMode is automatic, etc.
	// Fire(); // (optional) kick off shot logic here
}

void AShooterFirearm::Server_ReleaseTrigger_Implementation()
{
	IncreaseReleaseTrigger();

	// For automatic/semi-auto handling, stop timers, etc.
	// StopFire(); // (optional)
}

void AShooterFirearm::IncreasePressTrigger()
{
	// Mirrors BP node "IncreasePressTrigger": ++TriggerData.Pressed
	++TriggerData.Pressed;
}

void AShooterFirearm::IncreaseReleaseTrigger()
{
	// Mirrors BP node "IncreaseReleaseTrigger": ++TriggerData.Released
	++TriggerData.Released;
}

bool AShooterFirearm::CanPerformAction() const
{
	// Match the BP intent: allow if we have an owner, not reloading, etc.
	// And (optionally) pressed==released so first press starts a cycle.
	const bool bHasValidOwner = bHasOwner && (GetOwner() != nullptr);

	// Classic gate used in many input systems:
	// - allow a new "press" if pressed == released (i.e., not being held already)
	const bool bReadyForNewPress = (TriggerData.Pressed == TriggerData.Released);

	return bHasValidOwner && !bIsReloading && bReadyForNewPress;
}

// --- Optional stubs (fill these when you hook muzzle FX/trace/projectiles/anim) ---
void AShooterFirearm::Fire()
{
	if (!CurrentShooterPawnComponent) return;

	// Recoil
	CurrentShooterPawnComponent->PerformProceduralRecoil(
		FRotator(1.f), FVector(1.f), FRotator(1.f));

	// Play animations + effects
	PerformFireAnimation();
	PlayFireEffects();
	LaunchProjectile(FirearmComponent->GetMuzzleProjectileTransform(100.f, 1.f), true);

	// Auto fire loop
	if (CurrentFireMode == FGameplayTag::RequestGameplayTag(FName("Firearm.FireMode.Auto")))
	{
		GetWorldTimerManager().SetTimer(
			FullAutoTimerHandle, this, &AShooterFirearm::Fire, FireRateDelay, true);
	}
}

void AShooterFirearm::StopFire()
{
	GetWorldTimerManager().ClearTimer(FullAutoTimerHandle);
}

void AShooterFirearm::PerformFireAnimation()
{
	if (const UShooterPDAFirearm* PDA = Cast<UShooterPDAFirearm>(DAConstruction))
	{
		if (PDA->Animation_Firearm_Fire && FirearmMeshComponent)
		{
			FirearmMeshComponent->GetAnimInstance()->Montage_Play(PDA->Animation_Firearm_Fire);
		}
		if (PDA->Animation_Player_Fire && CurrentShooterPawnComponent)
		{
			if (USkeletalMeshComponent* PawnMesh = CurrentShooterPawnComponent->GetPawnMesh())
			{
				if (UAnimInstance* AnimInst = PawnMesh->GetAnimInstance())
				{
					AnimInst->Montage_Play(PDA->Animation_Player_Fire);
				}
			}
		}
	}
}

bool AShooterFirearm::IsDamagingProjectile(int32 ProjectileId) const
{
	return ProjectileId == DamagingProjectileId;
}

void AShooterFirearm::Server_LaunchProjectile_Implementation(const FSKGMuzzleTransform& LaunchTransform)
{
	// Server authoritative spawn path mirrors the BP hook.
	LaunchProjectile(LaunchTransform, false);
}