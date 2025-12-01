#include "Gameplay/Characters/ShooterCombatCharacter.h"
#include "Gameplay/Abilities/AttrSet_Combat.h"
#include "Gameplay/Combat/Weapons/Base/ShooterWeaponBase.h"
#include "Gameplay/Characters/Player/Movement/ShooterCharacterMovement_Doom.h"
#include "Gameplay/Tags/ShooterGameplayTags.h"

#include "Components/CapsuleComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayCueManager.h"
#include "GameplayTagsManager.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include <Components/SKGShooterPawnComponent.h>

AShooterCombatCharacter::AShooterCombatCharacter()
	: AShooterCombatCharacter(FObjectInitializer::Get())
{
}

AShooterCombatCharacter::AShooterCombatCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UShooterCharacterMovement_Doom>(
		ACharacter::CharacterMovementComponentName))
{
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	CombatAttributes = CreateDefaultSubobject<UAttrSet_Combat>(TEXT("AttrSet_Combat"));
	SKGShooterPawn = CreateDefaultSubobject<USKGShooterPawnComponent>(TEXT("SKGShooterPawn"));

	bReplicates = true;
	SetReplicateMovement(true);
}

void AShooterCombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("[ASC] BeginPlay() ASC=%p CombatAttributes=%p"),
		ASC,
		CombatAttributes
	);

	if (HasAuthority() && DefaultWeaponClass)
	{
		SpawnDefaultWeapon();
	}
}

void AShooterCombatCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (ASC)
	{
		ASC->InitAbilityActorInfo(this, this);

		if (CombatAttributes && !ASC->GetSpawnedAttributes().Contains(CombatAttributes))
		{
			ASC->AddSpawnedAttribute(CombatAttributes);
		}

		if (HasAuthority())
		{
			GrantStartupAbilities();
		}
	}
}

void AShooterCombatCharacter::GrantStartupAbilities()
{
	if (!ensure(GetLocalRole() == ROLE_Authority)) return;
	if (!ASC) return;

	if (bStartupAbilitiesGiven)
	{
		return;
	}

	// --- Give Fire ability (shared by AI and Player) ---
	if (FireWeaponAbilityClass)
	{
		FGameplayAbilitySpec Spec(FireWeaponAbilityClass, 1, INDEX_NONE, this);
		Spec.GetDynamicSpecSourceTags().AddTag(ShooterTags::Ability_Weapon_Fire);
		ASC->GiveAbility(Spec);

		UE_LOG(LogTemp, Warning, TEXT("[ASC] Granted FireWeaponAbility to %s"), *GetName());
	}

	bStartupAbilitiesGiven = true;
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
	return bIsDead;
}

void AShooterCombatCharacter::OnOutOfHealth()
{
	if (bIsDead) return;
	bIsDead = true;

	if (ASC)
	{
		const FGameplayTag DeathCue = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Damage.Death"));
		ASC->ExecuteGameplayCue(DeathCue);
	}

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
	UE_LOG(LogTemp, Warning, TEXT("[AI] HandleDeath() triggered for %s"), *GetNameSafe(this));

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}

	DisableInput(nullptr);

	if (ASC)
	{
		ASC->CancelAllAbilities();
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComp->SetSimulatePhysics(true);
		MeshComp->WakeAllRigidBodies();
		MeshComp->bBlendPhysics = true;
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (ASC)
	{
		const FGameplayTag DeathCue = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Damage.Death"));
		ASC->ExecuteGameplayCue(DeathCue);
	}

	const float CorpseLifetime = 8.0f;
	FTimerHandle CleanupHandle;
	GetWorldTimerManager().SetTimer(
		CleanupHandle,
		[this]()
		{
			Destroy();
		},
		CorpseLifetime,
		false
	);
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
	if (!DefaultWeaponClass || !GetMesh())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnDefaultWeapon_Internal] Missing DefaultWeaponClass or Mesh on %s"), *GetName());
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters P;
	P.Owner = this;
	P.Instigator = this; // ensure AI/projectiles have valid instigator
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AShooterWeaponBase* NewWeapon = World->SpawnActor<AShooterWeaponBase>(DefaultWeaponClass, FTransform::Identity, P);
	if (!NewWeapon)
	{
		return;
	}

	const FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, true);
	NewWeapon->AttachToComponent(GetMesh(), Rules, WeaponAttachSocket);

	EquippedWeapon = NewWeapon;
	NewWeapon->SetOwner(this);

	if (SKGShooterPawn && EquippedWeapon)
	{
		SKGShooterPawn->SetHeldActor(EquippedWeapon);
		EquippedWeapon->SetShooterPawn(SKGShooterPawn);
	}

	UE_LOG(LogTemp, Warning, TEXT("[SpawnDefaultWeapon_Internal] %s spawned and attached %s"), *GetNameSafe(this), *GetNameSafe(NewWeapon));
}

void AShooterCombatCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterCombatCharacter, bIsDead);
}
