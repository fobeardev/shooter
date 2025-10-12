// AShooterFirearm_Old.cpp

#include "ShooterFirearm_Old.h"
#include "Components/SKGShooterPawnComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "ShooterPDAFirearm_Old.h"
#include "Components/SkeletalMeshComponent.h"

AShooterFirearm_Old::AShooterFirearm_Old()
{
	bReplicates = true;
	// ReSharper disable once CppVirtualFunctionCallInsideCtor
	SetReplicateMovement(true);
}

void AShooterFirearm_Old::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterFirearm_Old, ReloadCounter);
	DOREPLIFETIME(AShooterFirearm_Old, bHasOwner);	
}

void AShooterFirearm_Old::BeginPlay()
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

void AShooterFirearm_Old::Tick(float DeltaTime)
{

	// --- Debug: Print out CurrentProceduralAnimComponent state ---
	if (FirearmComponent)
	{
		USKGProceduralAnimComponent* Proc = FirearmComponent->GetCurrentProceduralAnimComponent();
		if (Proc)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShooterFirearm: CurrentProceduralAnimComponent = %s (Owner: %s, Class: %s)"),
				*Proc->GetName(),
				*Proc->GetOwner()->GetName(),
				*Proc->GetClass()->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ShooterFirearm: CurrentProceduralAnimComponent is NULL!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ShooterFirearm: FirearmComponent is NULL!"));
	}
}


void AShooterFirearm_Old::OnDAConstructionBundlesLoaded()
{
	// BP: Cast DAConstruction -> PDA_Firearm and use FireRate/FireModes/AnimBP
	if (const UShooterPDAFirearm_Old* PDA = Cast<UShooterPDAFirearm_Old>(DAConstruction))
	{
		SetFireRateDelay(PDA->FireRate);
		SetupFireModes(PDA->FireModes);

		if (PDA->AnimBP && FirearmMeshComponent)
		{
			FirearmMeshComponent->SetAnimInstanceClass(PDA->AnimBP);
		}
	}
}

void AShooterFirearm_Old::SetOwningPawn(APawn* OwningPawn)
{
	// BP: SetOwner(OwningPawn)
	SetOwner(OwningPawn);

	// BP: CurrentShooterPawnComponent = GetShooterPawnComponent(OwningPawn)
	CurrentShooterPawnComponent = USKGShooterPawnComponent::GetShooterPawnComponent(OwningPawn);

	// BP: bHasOwner = IsValid(OwningPawn)
	bHasOwner = UKismetSystemLibrary::IsValid(OwningPawn);
	OnRep_HasOwner();
}

void AShooterFirearm_Old::Interact(APawn* InteractingPawn)
{
	// BP: SetReplicateMovement(false)
	SetReplicateMovement(false);
}

void AShooterFirearm_Old::Drop()
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

void AShooterFirearm_Old::SetupFireModes(const FGameplayTagContainer& FireModeTags)
{
	// BP: copy tags, set CurrentFireMode to first if available
	FireModes = FireModeTags;

	if (FireModes.Num() > 0)
	{
		CurrentFireMode = FireModes.GetByIndex(0);
	}
}

void AShooterFirearm_Old::SetFireRateDelay(double FireRate)
{
	// In the BP it passes PDA.FireRate (shots/sec).  Delay is seconds between shots.
	FireRateDelay = (FireRate > 0.0) ? (1.0f / static_cast<float>(FireRate)) : 0.0f;
}

void AShooterFirearm_Old::OnRep_HasOwner()
{
	// If you need UI/FX when ownership flips, do it here.
}

// ---- Shooting ----
void AShooterFirearm_Old::PressTrigger()
{
	// BP: if (CanPerformAction) { Switch Has Authority → (Remote) call Server; (Authority) call Server too is fine }
	if (!CanPerformAction())
	{
		return;
	}

	// Call the server to mutate state (even if we are authority, it's harmless and keeps one path)
	Server_PressTrigger();
}

void AShooterFirearm_Old::ReleaseTrigger()
{
	// BP releases compares TriggerData.Pressed > TriggerData.Released before doing work.
	// We'll do the same gate here (Reuse CanPerformAction for symmetry, or make a Release-specific check)
	if (TriggerData.Pressed <= TriggerData.Released)
	{
		return;
	}

	Server_ReleaseTrigger();
}

void AShooterFirearm_Old::Server_PressTrigger_Implementation()
{
	IncreasePressTrigger();

	// You likely want to start continuous fire if CurrentFireMode is automatic, etc.
	// Fire(); // (optional) kick off shot logic here
}

void AShooterFirearm_Old::Server_ReleaseTrigger_Implementation()
{
	IncreaseReleaseTrigger();

	// For automatic/semi-auto handling, stop timers, etc.
	// StopFire(); // (optional)
}

void AShooterFirearm_Old::IncreasePressTrigger()
{
	// Mirrors BP node "IncreasePressTrigger": ++TriggerData.Pressed
	++TriggerData.Pressed;
}

void AShooterFirearm_Old::IncreaseReleaseTrigger()
{
	// Mirrors BP node "IncreaseReleaseTrigger": ++TriggerData.Released
	++TriggerData.Released;
}

bool AShooterFirearm_Old::CanPerformAction() const
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
void AShooterFirearm_Old::Fire()
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
			FullAutoTimerHandle, this, &AShooterFirearm_Old::Fire, FireRateDelay, true);
	}
}

void AShooterFirearm_Old::StopFire()
{
	GetWorldTimerManager().ClearTimer(FullAutoTimerHandle);
}

void AShooterFirearm_Old::PerformFireAnimation()
{
	if (const UShooterPDAFirearm_Old* PDA = Cast<UShooterPDAFirearm_Old>(DAConstruction))
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

bool AShooterFirearm_Old::IsDamagingProjectile(int32 ProjectileId) const
{
	return ProjectileId == DamagingProjectileId;
}

void AShooterFirearm_Old::Server_LaunchProjectile_Implementation(const FSKGMuzzleTransform& LaunchTransform)
{
	// Server authoritative spawn path mirrors the BP hook.
	LaunchProjectile(LaunchTransform, false);
}

#pragma region Fire Mode

// BP: CycleFireMode (Interface event) — client calls into Server to commit on authority
void AShooterFirearm_Old::CycleFireMode()
{
	// Same pattern as other input → server paths
	Server_CycleFireMode();

	// It’s harmless to also do it on the authority path (PIE single player, listen-server owner)
	if (HasAuthority())
	{
		CurrentFireMode = GetNextFireMode();
	}
}

void AShooterFirearm_Old::Server_CycleFireMode_Implementation()
{
	// Server authoritative write mirrors the BP node chain:
	//   Event CycleFireMode → Server_CycleFireMode → Set CurrentFireMode(GetNextFireMode())
	CurrentFireMode = GetNextFireMode();
}

FGameplayTag AShooterFirearm_Old::GetNextFireMode() const
{
	// BP: GetNextFireMode — pick the next tag in FireModes; wrap around; fall back to first
	if (FireModes.Num() == 0)
	{
		return CurrentFireMode; // nothing to do
	}

	// Linear view of the container (order = insertion order)
	const TArray<FGameplayTag>& Modes = FireModes.GetGameplayTagArray();

	// Find current index (if not present, treat as "before first")
	int32 Index = Modes.IndexOfByKey(CurrentFireMode);
	if (Index == INDEX_NONE)
	{
		return Modes[0];
	}

	const int32 NextIndex = (Index + 1) % Modes.Num();
	return Modes[NextIndex];
}

#pragma endregion

#pragma region Reloading
// --- Reloading ---

void AShooterFirearm_Old::Reload()
{
	// BP: Event Reload -> Branch( CanPerformAction ) -> (true) ReleaseTrigger -> PerformReloadAnimations -> Server_Reload

	// Early out if action is blocked (owner missing, already reloading, etc.)
	if (!CanPerformAction())
	{
		return;
	}

	// Mirror the BP: release first so autofire timers stop cleanly
	ReleaseTrigger();

	// Kick local feedback (animations, sounds, etc.)
	PerformReloadAnimations();

	// Tell the server we tried to reload (safe to call from server as well)
	Server_Reload();
}

void AShooterFirearm_Old::Server_Reload_Implementation()
{
	// BP: Server_Reload
	// Spam guard the same way the BP increments a byte and compares
	// (their graph checks (ReloadCounter + 1) > 250). We’ll mimic that:
	const uint8 Next = static_cast<uint8>(ReloadCounter + 1);
	if (Next > 250)
	{
		ReloadCounter = 0; // wrap / reset like the BP’s false path
	}
	else
	{
		ReloadCounter = Next; // true path: set incremented value
	}

	// Server side: mark state; actual montages usually don’t run on dedicated server
	bIsReloading = true;

	// If you want server-authoritative reload logic (ammo refill etc.), do it here.
	// (Animations are purely cosmetic — keep them on owning client.)
}

void AShooterFirearm_Old::PerformReloadAnimations()
{
	// BP: PerformReloadAnimations -> GetReloadAnimations -> Branch(bValid)
	//  true: bIsReloading = true -> PlayMontage(PlayerMesh, PlayerMontage) -> PlayMontage(FirearmMesh, FirearmMontage)
	//  (then) OnBlendOut sets bIsReloading = false

	UAnimMontage* PlayerMontage = nullptr;
	UAnimMontage* FirearmMontage = nullptr;
	bool bValid = false;
	GetReloadAnimations(/*out*/ FirearmMontage, /*out*/ PlayerMontage, /*out*/ bValid);

	if (!bValid)
	{
		return;
	}

	bIsReloading = true;

	// Player montage
	if (CurrentShooterPawnComponent)
	{
		if (USkeletalMeshComponent* PawnMesh = CurrentShooterPawnComponent->GetPawnMesh())
		{
			if (UAnimInstance* Anim = PawnMesh->GetAnimInstance())
			{
				constexpr float PlayRate = 1.f;
				Anim->Montage_Play(PlayerMontage, PlayRate);
				// Bind to blend-out so we can clear the flag like the BP “On Blend Out”
				FOnMontageBlendingOutStarted BlendOut;
				BlendOut.BindUFunction(this, FName("OnReloadMontageBlendOut"));
				Anim->Montage_SetBlendingOutDelegate(BlendOut, PlayerMontage);
			}
		}
	}

	// Firearm montage (weapon skeletal mesh)
	if (FirearmMeshComponent && FirearmMeshComponent->GetAnimInstance())
	{
		UAnimInstance* Anim = FirearmMeshComponent->GetAnimInstance();
		constexpr float PlayRate = 1.f;
		Anim->Montage_Play(FirearmMontage, PlayRate);

		FOnMontageBlendingOutStarted BlendOut;
		BlendOut.BindUFunction(this, FName("OnReloadMontageBlendOut"));
		Anim->Montage_SetBlendingOutDelegate(BlendOut, FirearmMontage);
	}
}

void AShooterFirearm_Old::OnReloadMontageBlendOut(UAnimMontage* /*Montage*/, bool /*bInterrupted*/)
{
	// BP: bIsReloading = false on BlendOut path
	bIsReloading = false;
}

void AShooterFirearm_Old::GetReloadAnimations(UAnimMontage*& OutFirearm, UAnimMontage*& OutPlayer, bool& bValid) const
{
	OutFirearm = nullptr;
	OutPlayer = nullptr;
	bValid = false;

	// In the BP this comes from a data asset accessor "GetReloadAnimations".
	// Do the same here: pull from your PDA/DT or cached pointers if you have them.
	//
	// Example if you have a PDA with two fields:
	// if (const UShooterPDAFirearm_Old* PDA = Cast<UShooterPDAFirearm_Old>(DAConstruction))
	// {
	//     OutPlayer  = PDA->Animation_Player_Reload;
	//     OutFirearm = PDA->Animation_Firearm_Reload;
	// }

	// If you don’t have data yet, leave them null and bValid=false.
	// Flip this to true once both references are non-null.
	bValid = (OutPlayer != nullptr) || (OutFirearm != nullptr);
}

#pragma endregion