#include "Characters/ShooterCharacter.h"

// Engine / GAS
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffect.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include <Components/SKGShooterPawnComponent.h>
#include "Components/CapsuleComponent.h"
#include "InputActionValue.h"
#include "TimerManager.h"

// Project
#include "Abilities/AttrSet_Combat.h"
#include "Tags/ShooterGameplayTags.h"
#include "Weapons/ShooterWeaponBase.h"
#include "Firearms/ShooterFirearm.h"
#include "Game/ShooterGameMode.h"
#include <Kismet/GameplayStatics.h>

AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(100.f);
	SetMinNetUpdateFrequency(33.f);

	// Ensure mesh is attached to capsule (typical Character already is, but explicit is fine)
	GetMesh()->SetupAttachment(RootComponent);

	// Offset down so feet meet capsule base
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));

	// Rotate mesh to face +X forward (typical if model faces +Y)
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	// SKG Shooter Framework pawn component
	SKGShooterPawn = CreateDefaultSubobject<USKGShooterPawnComponent>(TEXT("SKGShooterPawn"));

	// Third person boom
	ThirdPersonBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonBoom"));
	ThirdPersonBoom->SetupAttachment(RootComponent);
	ThirdPersonBoom->TargetArmLength = ThirdPersonTargetArm;       // 300
	ThirdPersonBoom->SetRelativeLocation(ThirdPersonBoomOffset);   // (0,50,70)
	ThirdPersonBoom->bUsePawnControlRotation = true;
	ThirdPersonBoom->bEnableCameraLag = bThirdPersonLag;           // true
	ThirdPersonBoom->CameraLagSpeed = ThirdPersonLagSpeed;         // 12
	ThirdPersonBoom->CameraLagMaxDistance = ThirdPersonLagMaxDist; // 40
	ThirdPersonBoom->bInheritPitch = true;
	ThirdPersonBoom->bInheritYaw = true;
	ThirdPersonBoom->bInheritRoll = false;

	// Third person camera
	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->SetupAttachment(ThirdPersonBoom, USpringArmComponent::SocketName);
	ThirdPersonCamera->SetRelativeRotation(ThirdPersonCameraRelativeRot); // (0,-10,0)
	ThirdPersonCamera->bUsePawnControlRotation = false;
	ThirdPersonCamera->FieldOfView = ThirdPersonFOV;
	ThirdPersonCamera->SetAutoActivate(false);

	// First person camera attached to head socket
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), FirstPersonCameraSocket); // "S_Camera" by default
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->FieldOfView = FirstPersonFOV;

	// Movement orientation: aim with controller in FP; keep movement-driven yaw off
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 600.f;
		Move->MaxAcceleration = 2048.f;
		Move->BrakingDecelerationWalking = 2048.f;
		Move->GroundFriction = 8.f;

		Move->JumpZVelocity = 600.f;
		Move->GravityScale = 1.3f;
		Move->AirControl = 0.45f;
		Move->AirControlBoostMultiplier = 1.0f;
		Move->AirControlBoostVelocityThreshold = 0.f;

		Move->FallingLateralFriction = 0.8f;
		Move->BrakingDecelerationFalling = 800.f;

		Move->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;
		bUseControllerRotationPitch = false; // FP camera handles pitch
		bUseControllerRotationRoll = false;
	}

	// Charges defaults
	MaxDashCharges = 2;
	CurrentDashCharges = MaxDashCharges;
	Rep_CurrentDashCharges = CurrentDashCharges;

	// Recharge defaults
	bRefillAllAtOnce = true;
	RefillAllDelay = 0.80f;   // Hades-like
	DashRechargeDelay = 1.0f; // used only if bRefillAllAtOnce is false

	// Default to FP camera at spawn
	bUseThirdPersonCamera = false;

	UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: Ctor: Max=%d Curr=%d Delay=%.2f"), MaxDashCharges, CurrentDashCharges, DashRechargeDelay);
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AShooterCharacter, Rep_CurrentDashCharges, COND_OwnerOnly);
	DOREPLIFETIME(AShooterCharacter, bUseThirdPersonCamera);
}

void AShooterCharacter::OnRep_DashCharges()
{
	CurrentDashCharges = Rep_CurrentDashCharges;
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Auto-equip on server at start if a class is set
	if (HasAuthority() && DefaultFirearmClass)
	{
		SpawnDefaultFirearm_Internal();
	}

	ConfigureCameraDefaultsOnce();
	ApplyCameraMode();
	UpdateControllerPitchClamp();

	CurrentDashCharges = FMath::Clamp(CurrentDashCharges, 0, MaxDashCharges);
	UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: BeginPlay: Curr=%d/%d"), CurrentDashCharges, MaxDashCharges);

	if (ASC && GetLocalRole() == ROLE_Authority)
	{
		InitializeASC();
		// Grant in PossessedBy only (avoid double grant)
	}
}

void AShooterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (ASC && GetLocalRole() == ROLE_Authority)
	{
		InitializeASC();
		GrantStartupAbilities();
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
	if (ASC) { ASC->InitAbilityActorInfo(this, this); }
	ConfigureCameraDefaultsOnce();
	ApplyCameraMode();
	UpdateControllerPitchClamp();
}

void AShooterCharacter::Input_Look(const FVector2D& LookAxis)
{
	if (!Controller) return;
	const float DT = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
	AddControllerYawInput(LookAxis.X * MouseYawSensitivity * DT * 100.f);
	AddControllerPitchInput(LookAxis.Y * MousePitchSensitivity * DT * 100.f);
}

void AShooterCharacter::Input_Move(const FVector2D& MoveAxis)
{
	if (!Controller) return;
	const FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
	if (!FMath::IsNearlyZero(MoveAxis.Y)) AddMovementInput(Forward, MoveAxis.Y);
	if (!FMath::IsNearlyZero(MoveAxis.X)) AddMovementInput(Right, MoveAxis.X);
}

void AShooterCharacter::Input_Dash()
{
	UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: Input_Dash (Role=%d, Auth=%d)"), (int32)GetLocalRole(), HasAuthority() ? 1 : 0);

	if (HasAuthority())
	{
		if (ASC)
		{
			ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(ShooterTags::Ability_Movement_Dash));
		}
	}
	else
	{
		ServerTryActivateDash();
	}
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

// ------------------------------------------------------------
// NEW: GetEquippedWeapon
// ------------------------------------------------------------
AShooterWeaponBase* AShooterCharacter::GetEquippedWeapon() const
{
	return Cast<AShooterWeaponBase>(SpawnedFirearm);
}

// GAS setup
void AShooterCharacter::InitializeASC() { ASC->InitAbilityActorInfo(this, this); }

void AShooterCharacter::GrantStartupAbilities()
{
	if (!ensure(GetLocalRole() == ROLE_Authority)) return;
	if (!ASC) return;

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

	if (FireWeaponAbilityClass)
	{
		FGameplayAbilitySpec Spec(FireWeaponAbilityClass, 1, INDEX_NONE, this);
		Spec.GetDynamicSpecSourceTags().AddTag(ShooterTags::Ability_Weapon_Fire);
		ASC->GiveAbility(Spec);
	}

	bStartupAbilitiesGiven = true;
	ASC->InitAbilityActorInfo(this, this);
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
	if (bUseThirdPersonCamera)
	{
		if (ThirdPersonCamera) ThirdPersonCamera->SetActive(true);
		if (FirstPersonCamera) FirstPersonCamera->SetActive(false);

		// In TP, movement can orient rotation if desired
		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->bOrientRotationToMovement = true;
		}
		bUseControllerRotationYaw = false;
		bUseControllerRotationPitch = false;
	}
	else
	{
		if (FirstPersonCamera) FirstPersonCamera->SetActive(true);
		if (ThirdPersonCamera) ThirdPersonCamera->SetActive(false);

		if (UCharacterMovementComponent* Move = GetCharacterMovement())
		{
			Move->bOrientRotationToMovement = false;
		}
		bUseControllerRotationYaw = true;
		bUseControllerRotationPitch = false; // FP camera handles pitch
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

	// 3. Destroy equipped firearm
	if (SpawnedFirearm)
	{
		SpawnedFirearm->Destroy();
		SpawnedFirearm = nullptr;
	}

	// 4. Enable ragdoll on the mesh
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComp->SetSimulatePhysics(true);
		MeshComp->WakeAllRigidBodies();
		MeshComp->bBlendPhysics = true;
	}

	// 5. Disable capsule collision (so ragdoll doesn’t pop)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 6. Disable movement
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DisableMovement();
	}

	// 7. Trigger GameplayCue (optional)
	if (ASC)
	{
		ASC->ExecuteGameplayCue(ShooterTags::GameplayCue_Damage_Death);
	}

	// 8. Slow motion and fade to black
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.3f);

		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraFade(0.f, 1.f, 1.0f, FLinearColor::Black, false, true);
		}
	}

	// 9. Schedule respawn after short delay
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

void AShooterCharacter::SpawnDefaultFirearm()
{
	// Mirror your BP: if not authority, route to server; otherwise do it now
	if (!HasAuthority())
	{
		Server_SpawnDefaultFirearm();
		return;
	}
	SpawnDefaultFirearm_Internal();
}

void AShooterCharacter::Server_SpawnDefaultFirearm_Implementation()
{
	SpawnDefaultFirearm_Internal();
}

void AShooterCharacter::SpawnDefaultFirearm_Internal()
{
	if (!DefaultFirearmClass || !GetMesh())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Spawn params like the BP graph: owner = self, instigator = controller pawn
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = GetInstigator();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// We do not care about the initial transform since we snap to socket
	const FTransform SpawnTM = FTransform::Identity;

	AActor* NewFirearm = World->SpawnActor<AActor>(DefaultFirearmClass, SpawnTM, Params);
	if (!NewFirearm)
	{
		return;
	}

	// Attach to character mesh at socket, snap all, weld simulated bodies like the BP node
	const FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
	NewFirearm->AttachToComponent(GetMesh(), AttachRules, FirearmAttachSocket);

	// Keep optional local pointer
	SpawnedFirearm = NewFirearm;

	// Inform SKG component so procedurals/aiming can initialize and replicate to clients
	if (SKGShooterPawn)
	{
		SKGShooterPawn->SetHeldActor(NewFirearm);

		// --- Tell the firearm who its pawn is for recoil / procedurals ---
		if (AShooterFirearm* Firearm = Cast<AShooterFirearm>(NewFirearm))
		{
			Firearm->SetShooterPawn(SKGShooterPawn);
		}
	}
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
