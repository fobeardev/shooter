#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "ShooterCombatCharacter.generated.h"

class UAbilitySystemComponent;
class UAttrSet_Combat;
class AShooterWeaponBase;

/**
 * Base combat character class shared by player and AI.
 * Handles:
 *  - Ability System setup
 *  - Combat attributes (Health, Stamina)
 *  - OnDeath events and replication
 */
UCLASS()
class SHOOTER_API AShooterCombatCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AShooterCombatCharacter();

	// --- IAbilitySystemInterface ---
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// --- Health / Combat Interface ---
	UFUNCTION(BlueprintPure, Category = "Shooter|Combat")
	float GetHealth() const;

	UFUNCTION(BlueprintPure, Category = "Shooter|Combat")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Shooter|Combat")
	bool IsDead() const;

	// Death logic
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void OnOutOfHealth();

	// Weapons / Attacks
	UFUNCTION(BlueprintPure, Category = "Shooter|Weapon")
	AShooterWeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }

	UFUNCTION(BlueprintCallable, Category = "Shooter|Weapon")
	void SpawnDefaultWeapon();

protected:
	virtual void BeginPlay() override;

	// --- GAS Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|GAS")
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|GAS")
	TObjectPtr<UAttrSet_Combat> CombatAttributes;

	// --- Internal: health monitoring ---
	virtual void HandleHealthChanged(float NewHealth, float MaxHealth);
	virtual void HandleDeath();

	UFUNCTION()
	void OnRep_Death();

	UPROPERTY(ReplicatedUsing = OnRep_Death)
	bool bIsDead = false;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Weapons ---
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Weapon")
	TSubclassOf<AShooterWeaponBase> DefaultWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Weapon")
	FName WeaponAttachSocket = TEXT("ik_hand_gun");

	UPROPERTY(Transient)
	TObjectPtr<AShooterWeaponBase> EquippedWeapon = nullptr;

	UFUNCTION(Server, Reliable)
	void Server_SpawnDefaultWeapon();

	virtual void SpawnDefaultWeapon_Internal();   // shared spawn/attach
};
