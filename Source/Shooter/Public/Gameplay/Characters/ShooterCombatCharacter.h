#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "ShooterCombatCharacter.generated.h"

// Forward declarations
class UAbilitySystemComponent;
class UAttrSet_Combat;
class AShooterWeaponBase;
class USKGShooterPawnComponent;
class UGameplayAbility;

/**
 * Base combat-capable character (used by both Player and AI)
 * - Owns ASC and CombatAttributes
 * - Handles health, death, and weapon spawn
 * - Grants Fire ability to all inheritors
 */
UCLASS()
class SHOOTER_API AShooterCombatCharacter
	: public ACharacter
	, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AShooterCombatCharacter();
	explicit AShooterCombatCharacter(const FObjectInitializer& ObjectInitializer);

	// --- IAbilitySystemInterface ---
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	// --- Core Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* ASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAttrSet_Combat* CombatAttributes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|Components")
	USKGShooterPawnComponent* SKGShooterPawn;

	// --- Weapons ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooter|Weapons")
	TSubclassOf<AShooterWeaponBase> DefaultWeaponClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Shooter|Weapons")
	AShooterWeaponBase* EquippedWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooter|Weapons")
	FName WeaponAttachSocket = TEXT("ik_hand_gun");

	// --- Abilities ---
	/** Shared fire ability (granted to all subclasses, including AI) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	TSubclassOf<UGameplayAbility> FireWeaponAbilityClass;

	/** Flag to ensure abilities aren’t granted twice */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	bool bStartupAbilitiesGiven = false;

	// --- Health / State ---
	UPROPERTY(ReplicatedUsing = OnRep_Death, BlueprintReadOnly, Category = "Health")
	bool bIsDead = false;

	// --- GAS setup ---
	/** Grants shared startup abilities (fire, etc.) */
	virtual void GrantStartupAbilities();

public:
	// --- Lifecycle ---
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(BlueprintPure, Category = "Shooter|Weapons")
	AShooterWeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }

	// --- Combat / Health ---
	UFUNCTION(BlueprintPure)
	float GetHealth() const;

	UFUNCTION(BlueprintPure)
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure)
	bool IsDead() const;

	UFUNCTION()
	void OnOutOfHealth();

	UFUNCTION()
	void HandleHealthChanged(float NewHealth, float MaxHealth);

	UFUNCTION()
	virtual void HandleDeath();

	UFUNCTION()
	void OnRep_Death();

	// --- Weapon handling ---
	UFUNCTION(BlueprintCallable, Category = "Shooter|Weapons")
	void SpawnDefaultWeapon();

	UFUNCTION(Server, Reliable)
	void Server_SpawnDefaultWeapon();

	virtual void SpawnDefaultWeapon_Internal();

	// --- Replication ---
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
