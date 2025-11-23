#pragma once

#include "CoreMinimal.h"
#include "Characters/ShooterCombatCharacter.h"
#include "GameplayTagContainer.h"
#include "ShooterCharacter.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UCameraComponent;
class USpringArmComponent;
class UAugmentManagerComponent;
class AShooterWeaponBase;
class AActor;
struct FInputActionValue;

/**
 * Player-controlled shooter character.
 * Extends AShooterCombatCharacter with:
 *  - Camera system (FP/TP)
 *  - Dash abilities and charge logic
 *  - Firearm spawning / SKG integration
 *  - Player input handling
 */
UCLASS()
class SHOOTER_API AShooterCharacter : public AShooterCombatCharacter
{
	GENERATED_BODY()

public:
	AShooterCharacter(const FObjectInitializer& ObjectInitializer);

	// --- Input ---
	void Input_Look(const FVector2D& LookAxis);
	void Input_Move(const FVector2D& MoveAxis);
	void Input_JumpStart();
	void Input_JumpStop();
	void Input_Dash();
	void Input_Aim(const FInputActionValue& Value);
	void Input_FirePressed();
	void Input_FireReleased();

	// --- Camera ---
	UFUNCTION(BlueprintCallable, Category = "Shooter|Camera")
	void SetUseThirdPersonCamera(bool bEnable);

	UFUNCTION(BlueprintPure, Category = "Shooter|Camera")
	bool IsUsingThirdPersonCamera() const { return bUseThirdPersonCamera; }

	// --- Dash API ---
	UFUNCTION(BlueprintPure, Category = "Shooter|Dash|Charges")
	int32 GetCurrentDashCharges() const { return CurrentDashCharges; }

	UFUNCTION(BlueprintPure, Category = "Shooter|Dash|Charges")
	int32 GetMaxDashCharges() const { return MaxDashCharges; }

	UFUNCTION(BlueprintPure, Category = "Shooter|Dash|Charges")
	bool HasDashCharge() const { return CurrentDashCharges > 0; }

	bool ConsumeDashCharge();
	void EnsureDashRechargeRunning();
	void AddDashChargeLocal(int32 Delta)
	{
		CurrentDashCharges = FMath::Clamp(CurrentDashCharges + Delta, 0, MaxDashCharges);
	}

	UFUNCTION(BlueprintPure, Category = "Shooter|GAS")
	bool IsInIFrame() const;

	/** Debug: Apply self-damage via GAS for testing death/respawn */
	UFUNCTION(BlueprintCallable, Category = "Shooter|Debug")
	void Debug_ApplySelfDamage();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void SpawnDefaultWeapon_Internal() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void HandleDeath() override;

	// --- Abilities ---
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Abilities")
	TSubclassOf<UGameplayAbility> DashAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Abilities")
	TSubclassOf<UGameplayEffect> GE_DashIFrames;

	bool bStartupAbilitiesGiven = false;
	void InitializeASC();
	void GrantStartupAbilities();

	// --- Dash ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooter|Dash")
	int32 MaxDashCharges = 2;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Shooter|Dash")
	int32 CurrentDashCharges = 2;

	UPROPERTY(ReplicatedUsing = OnRep_DashCharges)
	int32 Rep_CurrentDashCharges = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooter|Dash")
	bool bRefillAllAtOnce = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooter|Dash", meta = (EditCondition = "bRefillAllAtOnce"))
	float RefillAllDelay = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooter|Dash")
	float DashRechargeDelay = 1.0f;

	UFUNCTION()
	void OnRep_DashCharges();

	FTimerHandle DashRechargeTimer;
	void DashRechargeTick_PerCharge();
	void OnRefillAllTimer();

	// --- Augment System ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Augments", meta = (AllowPrivateAccess = "true"))
	UAugmentManagerComponent* AugmentManager;

	// --- Camera ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Camera")
	TObjectPtr<USpringArmComponent> ThirdPersonBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Camera")
	TObjectPtr<UCameraComponent> ThirdPersonCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_UseThirdPersonCamera, Category = "Shooter|Camera")
	bool bUseThirdPersonCamera = false;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	FName FirstPersonCameraSocket = TEXT("S_Camera");

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	float FirstPersonFOV = 90.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	float ThirdPersonFOV = 90.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	float ThirdPersonTargetArm = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	FVector ThirdPersonBoomOffset = FVector(0.f, 50.f, 70.f);

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	FRotator ThirdPersonCameraRelativeRot = FRotator(0.f, -10.f, 0.f);

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	bool bThirdPersonLag = true;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	float ThirdPersonLagSpeed = 12.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	float ThirdPersonLagMaxDist = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooter|Camera")
	float MouseYawSensitivity = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooter|Camera")
	float MousePitchSensitivity = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooter|Camera")
	float MinPitch = -85.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooter|Camera")
	float MaxPitch = 85.f;

	UFUNCTION()
	void OnRep_UseThirdPersonCamera();

	void ApplyCameraMode();
	void ConfigureCameraDefaultsOnce();
	void UpdateControllerPitchClamp();

	UFUNCTION(Server, Reliable)
	void ServerSetUseThirdPersonCamera(bool bEnable);

	UFUNCTION(Server, Reliable)
	void ServerTryActivateDash();

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugMovementBasis = false;
};
