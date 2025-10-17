#include "Characters/ShooterCombatCharacter.h"
#include "Abilities/AttrSet_Combat.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

AShooterCombatCharacter::AShooterCombatCharacter()
{
	bReplicates = true;
	SetReplicateMovement(true);

	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	CombatAttributes = CreateDefaultSubobject<UAttrSet_Combat>(TEXT("AttrSet_Combat"));
}

void AShooterCombatCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (ASC)
	{
		ASC->InitAbilityActorInfo(this, this);
	}
}

UAbilitySystemComponent* AShooterCombatCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}

float AShooterCombatCharacter::GetHealth() const
{
	return CombatAttributes ? CombatAttributes->GetHealth() : 0.f;
}

float AShooterCombatCharacter::GetMaxHealth() const
{
	return CombatAttributes ? CombatAttributes->GetMaxHealth() : 0.f;
}

bool AShooterCombatCharacter::IsDead() const
{
	return bIsDead || (GetHealth() <= 0.f);
}

void AShooterCombatCharacter::HandleHealthChanged(float NewHealth, float MaxHealth)
{
	if (NewHealth <= 0.f && !bIsDead)
	{
		bIsDead = true;
		HandleDeath();
	}
}

void AShooterCombatCharacter::HandleDeath()
{
	SetLifeSpan(5.f);
}

void AShooterCombatCharacter::OnRep_Death()
{
	if (bIsDead)
	{
		HandleDeath();
	}
}

void AShooterCombatCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterCombatCharacter, bIsDead);
}
