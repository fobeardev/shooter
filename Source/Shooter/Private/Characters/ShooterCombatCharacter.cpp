#include "Characters/ShooterCombatCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Abilities/AttrSet_Combat.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayCueManager.h"
#include "GameplayTagsManager.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/ShooterWeaponBase.h"

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

		// --- Force-register our default subobject attribute set
		if (CombatAttributes)
		{
			ASC->AddSpawnedAttribute(CombatAttributes);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[ASC] Registered attribute sets for %s:"), *GetNameSafe(this));

	TArray<UAttributeSet*> AttributeSets = ASC->GetSpawnedAttributes();

	for (UAttributeSet* Set : AttributeSets)
	{
		UE_LOG(LogTemp, Log, TEXT(" - %s"), *Set->GetClass()->GetName());
	}

	if (HasAuthority() && DefaultWeaponClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnDefaultWeapon called for %s"), *GetName());
		SpawnDefaultWeapon();
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

void AShooterCombatCharacter::OnOutOfHealth()
{
	if (bIsDead) return;
	bIsDead = true;

	// Trigger death GameplayCue if available
	if (ASC)
	{
		FGameplayTag DeathCue = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Damage.Death"));
		ASC->ExecuteGameplayCue(DeathCue);
	}

	// Disable input and movement
	DisableInput(nullptr);
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HandleDeath();
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

void AShooterCombatCharacter::SpawnDefaultWeapon()
{
	if (HasAuthority()) 
	{ 
		SpawnDefaultWeapon_Internal(); 
	}
	else 
	{ 
		Server_SpawnDefaultWeapon(); 
	}
}

void AShooterCombatCharacter::Server_SpawnDefaultWeapon_Implementation()
{
	SpawnDefaultWeapon_Internal();
}

void AShooterCombatCharacter::SpawnDefaultWeapon_Internal()
{
	if (!DefaultWeaponClass || !GetMesh()) return;
	UWorld* World = GetWorld(); if (!World) return;

	FActorSpawnParameters P;
	P.Owner = this;
	P.Instigator = GetInstigator();
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AShooterWeaponBase* NewWeapon = World->SpawnActor<AShooterWeaponBase>(DefaultWeaponClass, FTransform::Identity, P);
	if (!NewWeapon) return;

	const FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, true);
	NewWeapon->AttachToComponent(GetMesh(), Rules, WeaponAttachSocket);
    
	EquippedWeapon = NewWeapon;
}

void AShooterCombatCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterCombatCharacter, bIsDead);
}
