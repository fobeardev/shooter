#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "ShooterCharacter.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;
class UAttrSet_Combat;
class UCameraComponent;
class USpringArmComponent;
class USKGShooterPawnComponent;

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AShooterCharacter();

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Camera and input
	UFUNCTION(BlueprintCallable, Category = "Shooter|Camera")
	void SetUseThirdPersonCamera(bool bEnable);

	UFUNCTION(BlueprintPure, Category = "Shooter|Camera")
	bool IsUsingThirdPersonCamera() const { return bUseThirdPersonCamera; }

	UFUNCTION(BlueprintCallable, Category = "Shooter|Input")
	void Input_Look(const FVector2D& LookAxis);
	UFUNCTION(BlueprintCallable, Category = "Shooter|Input")
	void Input_Move(const FVector2D& MoveAxis);
	UFUNCTION(BlueprintCallable, Category = "Shooter|Input")
	void Input_Dash();
	UFUNCTION(BlueprintCallable, Category = "Shooter|Input")
	void Input_Aim(const FInputActionValue& Value);
	UFUNCTION(BlueprintCallable, Category = "Shooter|GAS")  
	bool IsInIFrame() const;

	// Spawn default firearm (call on server or it will route to server)
	UFUNCTION(BlueprintCallable, Category = "Shooter|Firearm")
	void SpawnDefaultFirearm();
protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- GAS core ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|GAS")
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|GAS")
	TObjectPtr<UAttrSet_Combat> CombatAttributes;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Abilities")
	TSubclassOf<UGameplayAbility> DashAbilityClass;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Abilities")
	TSubclassOf<UGameplayEffect> GE_DashIFrames;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Tags")
	FGameplayTag Tag_Ability_Dash;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Tags")
	FGameplayTag Tag_State_IFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|SKG", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USKGShooterPawnComponent> SKGShooterPawn;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Tags")
	FGameplayTag Tag_State_Aiming;

	// --- Cameras ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Camera")
	TObjectPtr<USpringArmComponent> ThirdPersonBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Camera")
	TObjectPtr<UCameraComponent> ThirdPersonCamera;

	// Replicated camera mode for listen and dedicated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_UseThirdPersonCamera, Category = "Shooter|Camera")
	bool bUseThirdPersonCamera = false;

	// FP socket and FOV
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	FName FirstPersonCameraSocket = TEXT("S_Camera"); 

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	float FirstPersonFOV = 90.0f;

	// TP defaults
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	float ThirdPersonFOV = 90.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	float ThirdPersonTargetArm = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	FVector ThirdPersonBoomOffset = FVector(0.0f, 50.0f, 70.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	FRotator ThirdPersonCameraRelativeRot = FRotator(0.0f, -10.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	bool bThirdPersonLag = true;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	float ThirdPersonLagSpeed = 12.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Camera")
	float ThirdPersonLagMaxDist = 40.0f;

	// Sensitivity and pitch limits
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooter|Camera")
	float MouseYawSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooter|Camera")
	float MousePitchSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooter|Camera")
	float MinPitch = -85.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooter|Camera")
	float MaxPitch = 85.f;

	// --- Dash charges ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooter|Dash|Charges")
	int32 MaxDashCharges = 2;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Shooter|Dash|Charges")
	int32 CurrentDashCharges = 2;

	/** Legacy per-charge ticking delay (only used if bRefillAllAtOnce=false) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooter|Dash|Charges", meta = (EditCondition = "!bRefillAllAtOnce"))
	float DashRechargeDelay = 1.0f;

	/** Hades-like: after the last dash, wait this long, then refill ALL charges at once */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooter|Dash|Charges")
	bool bRefillAllAtOnce = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooter|Dash|Charges", meta = (EditCondition = "bRefillAllAtOnce"))
	float RefillAllDelay = 0.80f; // tune: 0.7-1.0s

	// Replicated to owner for UI
	UPROPERTY(ReplicatedUsing = OnRep_DashCharges)
	int32 Rep_CurrentDashCharges = 2;

	UFUNCTION()
	void OnRep_DashCharges();

	// Prevent duplicate ability grants
	UPROPERTY(VisibleInstanceOnly, Category = "Shooter|Abilities")
	bool bStartupAbilitiesGiven = false;

	// Class to spawn
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Firearm")
	TSubclassOf<AActor> DefaultFirearmClass;

	// Socket to attach to (matches your BP: ik_hand_gun)
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Firearm")
	FName FirearmAttachSocket = TEXT("ik_hand_gun");

	// Keep a reference locally if you want (not required for replication)
	UPROPERTY(Transient)
	TObjectPtr<AActor> SpawnedFirearm;

	// Server RPC that performs the spawn/attach and SetHeldActor
	UFUNCTION(Server, Reliable)
	void Server_SpawnDefaultFirearm();

	// Internal implementation used by server only
	void SpawnDefaultFirearm_Internal();

public:
	// --- Dash charge API ---
	UFUNCTION(BlueprintPure, Category = "Shooter|Dash|Charges")
	int32 GetCurrentDashCharges() const { return CurrentDashCharges; }

	UFUNCTION(BlueprintPure, Category = "Shooter|Dash|Charges")
	int32 GetMaxDashCharges() const { return MaxDashCharges; }

	UFUNCTION(BlueprintPure, Category = "Shooter|Dash|Charges")
	bool HasDashCharge() const { return CurrentDashCharges > 0; }

	bool ConsumeDashCharge();
	void EnsureDashRechargeRunning();       // starts or refreshes the correct timer

	// Client-only visual refund if Commit fails
	void AddDashChargeLocal(int32 Delta)
	{
		CurrentDashCharges = FMath::Clamp(CurrentDashCharges + Delta, 0, MaxDashCharges);
	}

protected:
	// Timers
	FTimerHandle DashRechargeTimer;         // used for BOTH modes
	void DashRechargeTick_PerCharge();      // legacy +1 tick
	void OnRefillAllTimer();                // Hades-like refill-to-max

	// GAS lifecycle
	void InitializeASC();
	void GrantStartupAbilities();

	// Dash RPC
	UFUNCTION(Server, Reliable)
	void ServerTryActivateDash();

	// Camera replication hook
	UFUNCTION()
	void OnRep_UseThirdPersonCamera();

	// Camera helpers
	void ApplyCameraMode();
	void ConfigureCameraDefaultsOnce();
	void UpdateControllerPitchClamp();

	// Camera mode RPC for correctness over network
	UFUNCTION(Server, Reliable)
	void ServerSetUseThirdPersonCamera(bool bEnable);
};
