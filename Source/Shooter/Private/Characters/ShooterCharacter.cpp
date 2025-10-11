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
#include "TimerManager.h"

// Project
#include "Abilities/AttrSet_Combat.h"

AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);
	NetUpdateFrequency = 100.f;
	MinNetUpdateFrequency = 33.f;

	// Ensure mesh is attached to capsule (typical Character already is, but explicit is fine)
	GetMesh()->SetupAttachment(RootComponent);

	// Offset down so feet meet capsule base
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));

	// Rotate mesh to face +X forward (typical if model faces +Y)
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	CombatAttributes = CreateDefaultSubobject<UAttrSet_Combat>(TEXT("AttrSet_Combat"));
	
	// SKG Shooter Framework pawn component
	SKGShooterPawn = CreateDefaultSubobject<USKGShooterPawnComponent>(TEXT("SKGShooterPawn"));

	// Third person boom
	ThirdPersonBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonBoom"));
	ThirdPersonBoom->SetupAttachment(RootComponent);
	ThirdPersonBoom->TargetArmLength = ThirdPersonTargetArm;      // 300
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
	FirstPersonCamera->SetupAttachment(GetMesh(), FirstPersonCameraSocket); // "head" by default
	FirstPersonCamera->SetRelativeLocation(FVector(10.f, 0.f, 10.f));
	FirstPersonCamera->SetRelativeRotation(FRotator::ZeroRotator);
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->FieldOfView = FirstPersonFOV;
	FirstPersonCamera->SetAutoActivate(true);

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

	Tag_Ability_Dash = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Dash"));
	Tag_State_IFrame = FGameplayTag::RequestGameplayTag(FName("State.IFrame"));

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

UAbilitySystemComponent* AShooterCharacter::GetAbilitySystemComponent() const { return ASC; }

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

	// Ensure FP camera snaps to the head socket at runtime even if mesh changes
	if (FirstPersonCamera && GetMesh())
	{
		FirstPersonCamera->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FirstPersonCameraSocket);
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
		if (ASC && Tag_Ability_Dash.IsValid())
		{
			ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(Tag_Ability_Dash));
		}
	}
	else
	{
		ServerTryActivateDash();
	}
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
		if (Tag_Ability_Dash.IsValid()) { Spec.DynamicAbilityTags.AddTag(Tag_Ability_Dash); }
		ASC->GiveAbility(Spec);
		UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: GrantStartupAbilities: Gave DashAbility"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Dash[Char]: GrantStartupAbilities: Skipped (already has dash)"));
	}

	bStartupAbilitiesGiven = true;
	ASC->InitAbilityActorInfo(this, this);
}

bool AShooterCharacter::IsInIFrame() const
{
	return ASC && Tag_State_IFrame.IsValid() && ASC->HasMatchingGameplayTag(Tag_State_IFrame);
}

void AShooterCharacter::ServerTryActivateDash_Implementation()
{
	if (ASC && Tag_Ability_Dash.IsValid())
	{
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(Tag_Ability_Dash));
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
		FirstPersonCamera->SetRelativeLocation(FVector(10.f, 0.f, 10.f));
		FirstPersonCamera->SetRelativeRotation(FRotator::ZeroRotator);
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
