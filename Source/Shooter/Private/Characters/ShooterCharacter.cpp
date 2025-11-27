#include "Characters/ShooterCharacter.h"

// Engine / GAS
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffect.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "InputActionValue.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include <Kismet/GameplayStatics.h>

// SKG Shooter Framework
#include <Components/SKGShooterPawnComponent.h>

// Project
#include "Abilities/AttrSet_Combat.h"
#include "Tags/ShooterGameplayTags.h"
#include "Game/ShooterGameMode.h"
#include "Weapons/Firearms/ShooterFirearm.h"
#include "Player/Components/ShooterCharacterMovement_Doom.h"
#include "Augments/AugmentManagerComponent.h"

AShooterCharacter::AShooterCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;
    SetReplicateMovement(true);
    SetNetUpdateFrequency(100.f);
    SetMinNetUpdateFrequency(33.f);

    // Ensure mesh is attached to capsule (typical Character already is)
    GetMesh()->SetupAttachment(RootComponent);
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));   // feet at capsule base
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));  // face +X

	AugmentManager = CreateDefaultSubobject<UAugmentManagerComponent>("AugmentManager");

    // -----------------------------
    // Third-person spring arm + camera
    // -----------------------------
    ThirdPersonBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonBoom"));
    ThirdPersonBoom->SetupAttachment(RootComponent);
    ThirdPersonBoom->TargetArmLength = ThirdPersonTargetArm;
    ThirdPersonBoom->SetRelativeLocation(ThirdPersonBoomOffset);
    ThirdPersonBoom->bUsePawnControlRotation = true;
    ThirdPersonBoom->bEnableCameraLag = bThirdPersonLag;
    ThirdPersonBoom->CameraLagSpeed = ThirdPersonLagSpeed;
    ThirdPersonBoom->CameraLagMaxDistance = ThirdPersonLagMaxDist;
    ThirdPersonBoom->bInheritPitch = true;
    ThirdPersonBoom->bInheritYaw = true;
    ThirdPersonBoom->bInheritRoll = false;

    ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
    ThirdPersonCamera->SetupAttachment(ThirdPersonBoom, USpringArmComponent::SocketName);
    ThirdPersonCamera->SetRelativeRotation(ThirdPersonCameraRelativeRot);
    ThirdPersonCamera->bUsePawnControlRotation = false;
    ThirdPersonCamera->FieldOfView = ThirdPersonFOV;
    ThirdPersonCamera->SetAutoActivate(false);

    // -----------------------------
    // First-person camera
    // -----------------------------
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetMesh(), FirstPersonCameraSocket);
    FirstPersonCamera->bUsePawnControlRotation = true;  // camera follows controller pitch/yaw
    FirstPersonCamera->FieldOfView = FirstPersonFOV;

    // -----------------------------
    // Controller rotation flags
    // -----------------------------
    bUseControllerRotationYaw = true;    // character faces aim direction
    bUseControllerRotationPitch = false; // camera handles pitch independently
    bUseControllerRotationRoll = false;

    // -----------------------------
    // Dash setup
    // -----------------------------
    MaxDashCharges = 2;
    CurrentDashCharges = MaxDashCharges;
    Rep_CurrentDashCharges = CurrentDashCharges;

    bRefillAllAtOnce = true;
    RefillAllDelay = 0.80f;
    DashRechargeDelay = 1.0f;

    // Default to first-person mode
    bUseThirdPersonCamera = false;

    UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: Ctor: Max=%d Curr=%d Delay=%.2f"), MaxDashCharges, CurrentDashCharges, DashRechargeDelay);
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AShooterCharacter, Rep_CurrentDashCharges, COND_OwnerOnly);
	DOREPLIFETIME(AShooterCharacter, bUseThirdPersonCamera);
	DOREPLIFETIME(AShooterCharacter, AugmentManager);
}

void AShooterCharacter::OnRep_DashCharges()
{
	CurrentDashCharges = Rep_CurrentDashCharges;
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	ConfigureCameraDefaultsOnce();
	ApplyCameraMode();
	UpdateControllerPitchClamp();

	CurrentDashCharges = FMath::Clamp(CurrentDashCharges, 0, MaxDashCharges);
	UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: BeginPlay: Curr=%d/%d"), CurrentDashCharges, MaxDashCharges);

	if (AugmentManager)
	{
		if (HasAuthority())
		{
			// Ensure component replication is active
			AugmentManager->SetIsReplicated(true);
		}
	}
}

void AShooterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (ASC)
	{
		InitializeASC();

		if (GetLocalRole() == ROLE_Authority)
		{
			GrantStartupAbilities();
		}
	}

	if (HasAuthority())
	{
		CurrentDashCharges = MaxDashCharges;
		Rep_CurrentDashCharges = CurrentDashCharges;
		UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: PossessedBy: Top-up -> Curr=%d/%d"), CurrentDashCharges, MaxDashCharges);
	}

	ConfigureCameraDefaultsOnce();
	ApplyCameraMode();
	UpdateControllerPitchClamp();

	// Fade back in
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, 0.8f, FLinearColor::Black, false, false);
		}
	}

}

void AShooterCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (ASC)
	{
		ASC->InitAbilityActorInfo(this, this);

		if (CombatAttributes && !ASC->GetSpawnedAttributes().Contains(CombatAttributes))
		{
			ASC->AddSpawnedAttribute(CombatAttributes);
		}
	}

	ConfigureCameraDefaultsOnce();
	ApplyCameraMode();
	UpdateControllerPitchClamp();
}

void AShooterCharacter::SpawnDefaultWeapon_Internal()
{
	Super::SpawnDefaultWeapon_Internal();
}

void AShooterCharacter::Input_Look(const FVector2D& LookAxis)
{
	if (!Controller) return;
	const float DT = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
	AddControllerYawInput(LookAxis.X * MouseYawSensitivity * DT * 100.f);
	AddControllerPitchInput(LookAxis.Y * MousePitchSensitivity * DT * 100.f);
}

static void DebugDrawBasis(UWorld* World, const FVector& Origin, const FVector& Fwd2D, const FVector& Right2D, FColor C = FColor::Cyan)
{
	const float Len = 100.f;
	DrawDebugDirectionalArrow(World, Origin, Origin + Fwd2D * Len, 16.f, FColor::Green, false, 0.1f, 0, 1.5f);
	DrawDebugDirectionalArrow(World, Origin, Origin + Right2D * Len, 16.f, FColor::Blue, false, 0.1f, 0, 1.5f);
	DrawDebugSphere(World, Origin, 4.f, 8, C, false, 0.1f);
}

void AShooterCharacter::Input_Move(const FVector2D& MoveAxis)
{
	if (!Controller) return;

	// 1) Read controller rotation (for debug) and camera forward
	const FRotator CR = Controller->GetControlRotation();

	if (bDebugMovementBasis)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Move] Yaw=%.3f Pitch=%.3f"), CR.Yaw, CR.Pitch);
	}

	FVector CamFwd = FVector::ForwardVector;

	if (FirstPersonCamera && FirstPersonCamera->IsActive())
	{
		CamFwd = FirstPersonCamera->GetForwardVector();
	}
	else if (ThirdPersonCamera && ThirdPersonCamera->IsActive())
	{
		CamFwd = ThirdPersonCamera->GetForwardVector();
	}
	else
	{
		CamFwd = CR.Vector(); // fallback
	}

	// 2) Project to XY plane
	FVector Fwd2D = CamFwd; Fwd2D.Z = 0.f;
	const bool bDegenerate = !Fwd2D.Normalize();

	// 3) If degenerate (near vertical) OR if projected forward points opposite
	// to the actor's facing (rare but can happen with certain rigs), rebase to actor yaw
	FVector SafeFwd = Fwd2D;
	{
		const FVector ActorFwd = GetActorForwardVector().GetSafeNormal2D();
		const float Dot = FVector::DotProduct(SafeFwd, ActorFwd);

		if (bDegenerate || Dot < 0.f)
		{
			const FRotator FlatYaw(0.f, CR.Yaw, 0.f);
			SafeFwd = FRotationMatrix(FlatYaw).GetUnitAxis(EAxis::X); // pure yaw forward
			UE_LOG(LogTemp, Warning, TEXT("[Move] Rebased to yaw-only basis (bDegenerate=%d, Dot=%.3f)"), bDegenerate ? 1 : 0, Dot);
		}
	}

	// 4) Build right from safe forward
	const FVector SafeRight = FVector::CrossProduct(FVector::UpVector, SafeFwd).GetSafeNormal();

	if (bDebugMovementBasis)
	{
		// 5) Log vectors to confirm signs
		UE_LOG(LogTemp, Warning, TEXT("[Move] CamFwd=(%.3f,%.3f,%.3f)  SafeFwd2D=(%.3f,%.3f,%.3f)  Right2D=(%.3f,%.3f,%.3f)  Axis=(%.3f,%.3f)"),
			CamFwd.X, CamFwd.Y, CamFwd.Z,
			SafeFwd.X, SafeFwd.Y, SafeFwd.Z,
			SafeRight.X, SafeRight.Y, SafeRight.Z,
			MoveAxis.X, MoveAxis.Y
		);

		// 6) Draw arrows at feet to visualize basis
		DebugDrawBasis(GetWorld(), GetActorLocation() + FVector(0, 0, 5), SafeFwd, SafeRight);
	}

	// 7) Apply input
	AddMovementInput(SafeFwd, MoveAxis.Y);
	AddMovementInput(SafeRight, MoveAxis.X);
}

void AShooterCharacter::Input_JumpStart()
{
	if (!GetCharacterMovement() || !GetCapsuleComponent())
	{
		UE_LOG(LogTemp, Error, TEXT("Input_JumpStart: Missing movement or capsule component"));
		return;
	}

	if (CanJump())
	{
		Jump();

		if (ASC)
		{
			//ASC->ExecuteGameplayCue(ShooterTags::GameplayCue_Movement_Jump);
		}
	}
}

void AShooterCharacter::Input_JumpStop()
{
	StopJumping();
}

void AShooterCharacter::Input_Dash()
{
	if (!ASC) return;

	UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: Input_Dash (Role=%d, Auth=%d)"), (int32)GetLocalRole(), HasAuthority() ? 1 : 0);
	ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(ShooterTags::Ability_Movement_Dash));

	//UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: Input_Dash (Role=%d, Auth=%d)"), (int32)GetLocalRole(), HasAuthority() ? 1 : 0);

	//if (HasAuthority())
	//{
	//	if (ASC)
	//	{
	//		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(ShooterTags::Ability_Movement_Dash));
	//	}
	//}
	//else
	//{
	//	ServerTryActivateDash();
	//}
}

// ------------------------------------------------------------
// Input_Aim (Enhanced Input call via ShooterPlayerController)
// ------------------------------------------------------------
void AShooterCharacter::Input_Aim(const FInputActionValue& Value)
{
	const bool bPressed = Value.Get<bool>();

	if (!SKGShooterPawn)
		return;

	UE_LOG(LogTemp, Warning, TEXT("AIM INPUT: HeldActor=%s, FirearmComp=%s, ProceduralComp=%s"),
		*GetNameSafe(SKGShooterPawn->GetHeldActor()),
		*GetNameSafe(SKGShooterPawn->GetCurrentFirearmComponent()),
		*GetNameSafe(SKGShooterPawn->GetCurrentProceduralAnimComponent()));

	if (bPressed)
	{
		SKGShooterPawn->StartAiming();
	}
	else
	{
		SKGShooterPawn->StopAiming();
	}

	if (ASC)
	{
		if (bPressed)
		{
			ASC->AddLooseGameplayTag(ShooterTags::State_ADS);
		}
		else
		{
			ASC->RemoveLooseGameplayTag(ShooterTags::State_ADS);
		}

		bool bHasTag = ASC->HasMatchingGameplayTag(ShooterTags::State_ADS);
		UE_LOG(LogTemp, Warning, TEXT("Aiming Tag Active: %d"), bHasTag);
	}
}

// ------------------------------------------------------------
// NEW: Input_FirePressed / Input_FireReleased
// ------------------------------------------------------------
void AShooterCharacter::Input_FirePressed()
{
	if (!ASC) return;

	// Try to activate the weapon fire ability (mapped to Ability.Weapon.Fire)
	ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(ShooterTags::Ability_Weapon_Fire));
}

void AShooterCharacter::Input_FireReleased()
{
	if (!ASC) return;

	// Stop any active fire ability
	FGameplayTagContainer TagsToCancel;
	TagsToCancel.AddTag(ShooterTags::Ability_Weapon_Fire);
	ASC->CancelAbilities(&TagsToCancel);
}

void AShooterCharacter::Input_Interact()
{
	UE_LOG(LogTemp, Error, TEXT("Character Input_Interact reached!"));

	if (ASC)
	{
		ASC->TryActivateAbilitiesByTag(
			FGameplayTagContainer(ShooterTags::Ability_Utility_Interact)
		);
	}
}

// GAS setup
void AShooterCharacter::InitializeASC()
{
	if (!ASC) return;

	// Avoid reinitializing if already valid
	if (ASC->AbilityActorInfo.IsValid() && ASC->AbilityActorInfo->AvatarActor == this)
		return;

	ASC->InitAbilityActorInfo(this, this);

	UE_LOG(LogTemp, Log, TEXT("[ASC] Initialized for %s (Role=%s)"),
		*GetNameSafe(this),
		*UEnum::GetValueAsString(GetLocalRole()));
}


void AShooterCharacter::GrantStartupAbilities()
{
	if (!ensure(GetLocalRole() == ROLE_Authority)) return;
	if (!ASC) return;

	Super::GrantStartupAbilities();

	if (bStartupAbilitiesGiven)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: GrantStartupAbilities: already given (flag)"));
		return;
	}

	bool bAlreadyHasDash = false;
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetClass() == DashAbilityClass)
		{
			bAlreadyHasDash = true;
			break;
		}
	}

	if (!bAlreadyHasDash && DashAbilityClass)
	{
		FGameplayAbilitySpec Spec(DashAbilityClass, 1, INDEX_NONE, this);
		Spec.GetDynamicSpecSourceTags().AddTag(ShooterTags::Ability_Movement_Dash);
		ASC->GiveAbility(Spec);
		UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: GrantStartupAbilities: Gave DashAbility"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: GrantStartupAbilities: Skipped (already has dash)"));
	}

	// --- Interact Ability ---
	bool bHasInteract = false;
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetClass() == InteractAbilityClass)
		{
			bHasInteract = true;
			break;
		}
	}

	if (!bHasInteract && InteractAbilityClass)
	{
		FGameplayAbilitySpec InteractSpec(InteractAbilityClass, 1, INDEX_NONE, this);

		// Tag the spec so ASC->TryActivateAbilitiesByTag() works
		InteractSpec.GetDynamicSpecSourceTags().AddTag(ShooterTags::Ability_Utility_Interact);

		ASC->GiveAbility(InteractSpec);

		UE_LOG(LogTemp, Warning, TEXT("StartupAbilities: Granted InteractAbility."));
	}


	bStartupAbilitiesGiven = true;
}

bool AShooterCharacter::IsInIFrame() const
{
	return ASC && ASC->HasMatchingGameplayTag(ShooterTags::State_IFrame);
}

void AShooterCharacter::ServerTryActivateDash_Implementation()
{
	if (ASC)
	{
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(ShooterTags::Ability_Movement_Dash));
	}
}

// --- Charges -------------------------------------------------------------------------------------

bool AShooterCharacter::ConsumeDashCharge()
{
	if (HasAuthority())
	{
		if (CurrentDashCharges <= 0) return false;
		--CurrentDashCharges;
		Rep_CurrentDashCharges = CurrentDashCharges;
		UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: ConsumeDashCharge OK (server): Curr=%d/%d"), CurrentDashCharges, MaxDashCharges);

		EnsureDashRechargeRunning();
		return true;
	}

	if (CurrentDashCharges <= 0) return false;
	--CurrentDashCharges;
	UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: ConsumeDashCharge (client predict): Curr=%d/%d"), CurrentDashCharges, MaxDashCharges);
	return true;
}

void AShooterCharacter::EnsureDashRechargeRunning()
{
	if (!HasAuthority()) return;
	if (CurrentDashCharges >= MaxDashCharges) return;

	// Hades-like: single refill to max after a short delay from the last dash
	if (bRefillAllAtOnce)
	{
		GetWorldTimerManager().ClearTimer(DashRechargeTimer);
		GetWorldTimerManager().SetTimer(
			DashRechargeTimer,
			this,
			&AShooterCharacter::OnRefillAllTimer,
			FMath::Max(0.01f, RefillAllDelay),
			false);

		UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: RefillAll timer (%.2fs) scheduled/reset. Curr=%d/%d"),
			RefillAllDelay, CurrentDashCharges, MaxDashCharges);
		return;
	}

	// Legacy per-charge ticking
	if (DashRechargeDelay <= 0.f) { DashRechargeDelay = 1.0f; }

	GetWorldTimerManager().SetTimer(
		DashRechargeTimer,
		this,
		&AShooterCharacter::DashRechargeTick_PerCharge,
		DashRechargeDelay,
		true);

	UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: EnsureDashRechargeRunning: Start per-charge timer (Delay=%.2f) Curr=%d/%d"),
		DashRechargeDelay, CurrentDashCharges, MaxDashCharges);
}

void AShooterCharacter::OnRefillAllTimer()
{
	if (!HasAuthority()) return;

	CurrentDashCharges = MaxDashCharges;
	Rep_CurrentDashCharges = CurrentDashCharges;
	GetWorldTimerManager().ClearTimer(DashRechargeTimer);

	UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: RefillAllTimer fired -> Curr=%d/%d (refilled to max)"),
		CurrentDashCharges, MaxDashCharges);
}

void AShooterCharacter::DashRechargeTick_PerCharge()
{
	if (!HasAuthority()) return;

	if (CurrentDashCharges < MaxDashCharges)
	{
		++CurrentDashCharges;
		Rep_CurrentDashCharges = CurrentDashCharges;
		UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: DashRechargeTick: +1 -> Curr=%d/%d"), CurrentDashCharges, MaxDashCharges);
	}

	if (CurrentDashCharges >= MaxDashCharges)
	{
		GetWorldTimerManager().ClearTimer(DashRechargeTimer);
	}
}

// --- Camera helpers ------------------------------------------------------------------------------

void AShooterCharacter::SetUseThirdPersonCamera(bool bEnable)
{
	if (bUseThirdPersonCamera == bEnable)
	{
		return;
	}

	if (HasAuthority())
	{
		bUseThirdPersonCamera = bEnable;
		ApplyCameraMode();
	}
	else
	{
		// Optimistic local switch for snappy feel, server will replicate truth
		bUseThirdPersonCamera = bEnable;
		ApplyCameraMode();
		ServerSetUseThirdPersonCamera(bEnable);
	}
}

void AShooterCharacter::ServerSetUseThirdPersonCamera_Implementation(bool bEnable)
{
	bUseThirdPersonCamera = bEnable;
	ApplyCameraMode();
}

void AShooterCharacter::OnRep_UseThirdPersonCamera()
{
	ApplyCameraMode();
}
void AShooterCharacter::ApplyCameraMode()
{
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		// Always decouple base rotation to prevent weird pitch carryover
		Move->bIgnoreBaseRotation = true;
	}

	if (bUseThirdPersonCamera)
	{
		if (ThirdPersonCamera) ThirdPersonCamera->SetActive(true);
		if (FirstPersonCamera) FirstPersonCamera->SetActive(false);

		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->bOrientRotationToMovement = true;
			Move->bUseControllerDesiredRotation = false;
		}

		bUseControllerRotationYaw = false;
	}
	else
	{
		if (FirstPersonCamera) FirstPersonCamera->SetActive(true);
		if (ThirdPersonCamera) ThirdPersonCamera->SetActive(false);

		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->bOrientRotationToMovement = false;
			Move->bUseControllerDesiredRotation = true;
			Move->bIgnoreBaseRotation = true;

			// Keep consistent air control and prevent braking midair
			Move->AirControl = 1.0f;
			Move->BrakingDecelerationFalling = 0.f;
			Move->FallingLateralFriction = 0.f;
		}

		bUseControllerRotationYaw = true;
	}

	// Enforce correct orientation even if mid-jump
	if (Controller)
	{
		FRotator ControlRot = Controller->GetControlRotation();
		ControlRot.Pitch = 0.f;
		ControlRot.Roll = 0.f;
		SetActorRotation(ControlRot);
	}
}

void AShooterCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// Optional cue
	if (ASC)
	{
		//ASC->ExecuteGameplayCue(ShooterTags::GameplayCue_Movement_Land);
	}
}

void AShooterCharacter::ConfigureCameraDefaultsOnce()
{
	// Keep TP values in sync with editable defaults
	if (ThirdPersonBoom)
	{
		ThirdPersonBoom->TargetArmLength = ThirdPersonTargetArm;
		ThirdPersonBoom->SetRelativeLocation(ThirdPersonBoomOffset);
		ThirdPersonBoom->bUsePawnControlRotation = true;
		ThirdPersonBoom->bEnableCameraLag = bThirdPersonLag;
		ThirdPersonBoom->CameraLagSpeed = ThirdPersonLagSpeed;
		ThirdPersonBoom->CameraLagMaxDistance = ThirdPersonLagMaxDist;
		ThirdPersonBoom->bInheritPitch = true;
		ThirdPersonBoom->bInheritYaw = true;
		ThirdPersonBoom->bInheritRoll = false;
	}
	if (ThirdPersonCamera)
	{
		ThirdPersonCamera->SetRelativeRotation(ThirdPersonCameraRelativeRot);
		ThirdPersonCamera->bUsePawnControlRotation = false;
		ThirdPersonCamera->FieldOfView = ThirdPersonFOV;
	}
	if (FirstPersonCamera)
	{
		FirstPersonCamera->bUsePawnControlRotation = true;
		FirstPersonCamera->FieldOfView = FirstPersonFOV;
	}
}

void AShooterCharacter::UpdateControllerPitchClamp()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->PlayerCameraManager->ViewPitchMin = MinPitch;
		PC->PlayerCameraManager->ViewPitchMax = MaxPitch;
	}
}

void AShooterCharacter::HandleDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("HandleDeath() triggered for %s"), *GetName());

	SetUseThirdPersonCamera(true);
	ApplyCameraMode();

	// 1. Disable input (client side)
	if (AController* C = GetController())
	{
		C->SetIgnoreMoveInput(true);
		C->SetIgnoreLookInput(true);
	}

	// 2. Stop all abilities
	if (ASC)
	{
		ASC->CancelAllAbilities();
	}

	// 3. Enable ragdoll on the mesh
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComp->SetSimulatePhysics(true);
		MeshComp->WakeAllRigidBodies();
		MeshComp->bBlendPhysics = true;
	}

	// 4. Disable capsule collision (so ragdoll doesn’t pop)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 5. Disable movement
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DisableMovement();
	}

	// 6. Trigger GameplayCue (optional)
	if (ASC)
	{
		ASC->ExecuteGameplayCue(ShooterTags::GameplayCue_Damage_Death);
	}

	// 7. Slow motion and fade to black
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.3f);

		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraFade(0.f, 1.f, 1.0f, FLinearColor::Black, false, true);
		}
	}

	// 8. Schedule respawn after short delay
	if (HasAuthority())
	{
		FTimerHandle RespawnHandle;
		GetWorldTimerManager().SetTimer(
			RespawnHandle,
			[this]()
			{
				UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

				AController* ControllerRef = GetController();
				if (!ControllerRef)
				{
					UE_LOG(LogTemp, Error, TEXT("HandleDeath: No controller found."));
					return;
				}

				AShooterGameMode* GM = GetWorld()->GetAuthGameMode<AShooterGameMode>();
				if (!GM)
				{
					UE_LOG(LogTemp, Error, TEXT("HandleDeath: No GameMode found."));
					return;
				}

				AShooterCharacter* OldCharacter = this;

				UE_LOG(LogTemp, Warning, TEXT("HandleDeath: Unpossessing old pawn %s"), *GetName());
				ControllerRef->UnPossess(); // <-- critical line

				// Now spawn the new pawn cleanly
				UE_LOG(LogTemp, Warning, TEXT("HandleDeath: Restarting player..."));
				GM->RestartPlayer(ControllerRef);

				APawn* NewPawn = ControllerRef->GetPawn();
				if (!NewPawn)
				{
					UE_LOG(LogTemp, Error, TEXT("HandleDeath: Controller has no new pawn after RestartPlayer."));
					return;
				}

				UE_LOG(LogTemp, Warning, TEXT("HandleDeath: Controller now possesses %s"), *GetNameSafe(NewPawn));

				// Hide and destroy the old pawn
				if (OldCharacter && OldCharacter != NewPawn)
				{
					if (USkeletalMeshComponent* MeshComp = OldCharacter->FindComponentByClass<USkeletalMeshComponent>())
					{
						MeshComp->SetHiddenInGame(true);
					}

					OldCharacter->Destroy();
					UE_LOG(LogTemp, Warning, TEXT("HandleDeath: Old pawn destroyed."));
				}

				// Fade-in from black on respawn
				if (APlayerController* PC = Cast<APlayerController>(ControllerRef))
				{
					if (PC->PlayerCameraManager)
					{
						PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, 0.8f, FLinearColor::Black, false, false);
					}
				}
			},
			2.5f,
			false
		);
	}
}

AActor* AShooterCharacter::PerformInteractionTrace(float Distance, FHitResult& OutHit)
{
	OutHit = FHitResult();

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return nullptr;
	}

	APlayerCameraManager* Cam = PC->PlayerCameraManager;
	if (!Cam)
	{
		return nullptr;
	}

	const FVector Start = Cam->GetCameraLocation();
	const FVector End = Start + (Cam->GetActorForwardVector() * Distance);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractTrace), false, this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		ECC_Visibility,
		Params
	);

	return bHit ? OutHit.GetActor() : nullptr;
}

void AShooterCharacter::Debug_ApplySelfDamage()
{
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("Debug_ApplySelfDamage: No ASC"));
		return;
	}

	// Load the GE class (use LoadClass, not LoadObject)
	static TSubclassOf<UGameplayEffect> DamageEffectClass = LoadClass<UGameplayEffect>(
		nullptr,
		TEXT("/Game/Hadeslike/Abilities/GE/GE_Damage_Ballistic.GE_Damage_Ballistic_C")
	);

	if (!DamageEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Debug_ApplySelfDamage: Could not find GE_Damage_Ballistic class"));
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, Context);
	if (SpecHandle.IsValid() && SpecHandle.Data.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Weapon.DamageScalar"), -99999.0f);
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		UE_LOG(LogTemp, Warning, TEXT("Debug_ApplySelfDamage: Applied lethal self-damage via GAS (using GE_Damage_Ballistic)"));
	}
}
