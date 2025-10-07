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

/**
 * Networked character that owns GAS (ASC + AttributeSet),
 * can activate Dash via GAS, and exposes an I-Frame query.
 *
 * Dash execution, i-frames application, FX and cooldown
 * should be implemented inside your UGameplayAbility subclass.
 */
UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AShooterCharacter();

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// True while a Dash i-frame GameplayEffect/Tag is active on this pawn.
	UFUNCTION(BlueprintCallable, Category = "Shooter|GAS")
	bool IsInIFrame() const;

	// Local input helper (PC calls this on dash input).
	UFUNCTION(BlueprintCallable, Category = "Shooter|Input")
	void Input_Dash();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// GAS core
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|GAS")
	TObjectPtr<UAbilitySystemComponent> ASC;

	// Primary AttributeSet (Health/MoveSpeed etc.)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shooter|GAS")
	TObjectPtr<UAttrSet_Combat> CombatAttributes;

	// Given on spawn/possess (server only). Should point to your UGameplayAbility Dash class.
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Abilities")
	TSubclassOf<UGameplayAbility> DashAbilityClass;

	// Optional: i-frame effect if you also want to apply from Character (normally applied by the ability).
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Abilities")
	TSubclassOf<UGameplayEffect> GE_DashIFrames;

	// Tags (set in defaults or via config).
	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Tags")
	FGameplayTag Tag_Ability_Dash;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter|Tags")
	FGameplayTag Tag_State_IFrame;

	// GAS lifecycle
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	// Server tries to activate the Dash ability by tag.
	UFUNCTION(Server, Reliable)
	void ServerTryActivateDash();

	// Initialize ASC actor info & grant startup abilities (server). Safe to call on possess and onrep.
	void InitializeASC();
	void GrantStartupAbilities();

	// (Optional) stop firing/ADS during dash start; useful if you want clean animation/feel.
	void PauseFirearmsDuringDash(float PauseForSeconds = 0.2f);
};
