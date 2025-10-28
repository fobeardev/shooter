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
#include <Components/SKGShooterPawnComponent.h>
#include <Player/Components/ShooterCharacterMovement_Doom.h>

AShooterCombatCharacter::AShooterCombatCharacter()
	: AShooterCombatCharacter(FObjectInitializer::Get())
{
}

AShooterCombatCharacter::AShooterCombatCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UShooterCharacterMovement_Doom>(
		ACharacter::CharacterMovementComponentName))
{
	bReplicates = true;
	SetReplicateMovement(true);

	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	CombatAttributes = CreateDefaultSubobject<UAttrSet_Combat>(TEXT("AttrSet_Combat"));

	// SKG Shooter Framework pawn component
	SKGShooterPawn = CreateDefaultSubobject<USKGShooterPawnComponent>(TEXT("SKGShooterPawn"));
}

void AShooterCombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	// No InitAbilityActorInfo here
	UE_LOG(LogTemp, Log, TEXT("[ASC] BeginPlay() ASC=%s CombatAttributes=%s"),
		*GetNameSafe(ASC),
		*GetNameSafe(CombatAttributes));

	UE_LOG(LogTemp, Warning, TEXT("Controller=%s  IsLocallyControlled=%d"),
		*GetNameSafe(Controller),
		IsLocallyControlled() ? 1 : 0);

	if (HasAuthority() && DefaultWeaponClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnDefaultWeapon called for %s"), *GetName());
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
	return bIsDead;
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
	UE_LOG(LogTemp, Warning, TEXT("[AI] HandleDeath() triggered for %s"), *GetNameSafe(this));

	// 1. Stop movement & disable input
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}

	DisableInput(nullptr);

	// 2. Stop abilities
	if (ASC)
	{
		ASC->CancelAllAbilities();
	}

	// 3. Switch to ragdoll physics on the mesh
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComp->SetSimulatePhysics(true);
		MeshComp->WakeAllRigidBodies();
		MeshComp->bBlendPhysics = true;
	}

	// 4. Disable capsule collision so the body doesn't pop up
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 5. Play optional death cue or VFX
	if (ASC)
	{
		FGameplayTag DeathCue = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Damage.Death"));
		ASC->ExecuteGameplayCue(DeathCue);
	}

	// 6. Schedule cleanup (destroy) after a few seconds
	const float CorpseLifetime = 8.0f;
	FTimerHandle CleanupHandle;
	GetWorldTimerManager().SetTimer(
		CleanupHandle,
		[this]()
		{
			UE_LOG(LogTemp, Warning, TEXT("[AI] Destroying ragdoll corpse: %s"), *GetNameSafe(this));
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

//void AShooterCombatCharacter::SpawnDefaultWeapon_Internal()
//{
//	if (!DefaultWeaponClass || !GetMesh()) return;
//	UWorld* World = GetWorld(); if (!World) return;
//
//	FActorSpawnParameters P;
//	P.Owner = this;
//	P.Instigator = GetInstigator();
//	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
//
//	AShooterWeaponBase* NewWeapon = World->SpawnActor<AShooterWeaponBase>(DefaultWeaponClass, FTransform::Identity, P);
//	if (!NewWeapon) return;
//
//	const FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, true);
//	NewWeapon->AttachToComponent(GetMesh(), Rules, WeaponAttachSocket);
//    
//	EquippedWeapon = NewWeapon;
//}

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
		UE_LOG(LogTemp, Warning, TEXT("[SpawnDefaultWeapon_Internal] No valid World for %s"), *GetName());
		return;
	}

	FActorSpawnParameters P;
	P.Owner = this;
	P.Instigator = GetInstigator();
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AShooterWeaponBase* NewWeapon = World->SpawnActor<AShooterWeaponBase>(DefaultWeaponClass, FTransform::Identity, P);
	if (!NewWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnDefaultWeapon_Internal] Failed to spawn weapon for %s"), *GetName());
		return;
	}

	const FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, true);
	NewWeapon->AttachToComponent(GetMesh(), Rules, WeaponAttachSocket);

	EquippedWeapon = NewWeapon;

	// Debug log chain
	UE_LOG(LogTemp, Warning, TEXT("[SpawnDefaultWeapon_Internal] %s spawned %s and attached to socket '%s'"),
		*GetNameSafe(this),
		*GetNameSafe(NewWeapon),
		*WeaponAttachSocket.ToString());

	// Check attachment validity
	if (NewWeapon->GetAttachParentActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnDefaultWeapon_Internal] Weapon %s is attached to actor %s"),
			*GetNameSafe(NewWeapon),
			*GetNameSafe(NewWeapon->GetAttachParentActor()));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SpawnDefaultWeapon_Internal] Weapon %s is NOT attached to any actor"), *GetNameSafe(NewWeapon));
	}

	// Optional: confirm mesh bone/socket existence
	if (!GetMesh()->DoesSocketExist(WeaponAttachSocket))
	{
		UE_LOG(LogTemp, Error, TEXT("[SpawnDefaultWeapon_Internal] Socket '%s' not found on mesh %s for %s"),
			*WeaponAttachSocket.ToString(),
			*GetNameSafe(GetMesh()),
			*GetName());
	}

	// Optional: ensure transform correctness
	FTransform SocketTransform = GetMesh()->GetSocketTransform(WeaponAttachSocket);
	UE_LOG(LogTemp, Warning, TEXT("[SpawnDefaultWeapon_Internal] Socket location (%.1f, %.1f, %.1f) rotation (%.1f, %.1f, %.1f)"),
		SocketTransform.GetLocation().X, SocketTransform.GetLocation().Y, SocketTransform.GetLocation().Z,
		SocketTransform.Rotator().Pitch, SocketTransform.Rotator().Yaw, SocketTransform.Rotator().Roll);

	if (SKGShooterPawn && EquippedWeapon)
	{
		SKGShooterPawn->SetHeldActor(EquippedWeapon);

		if (EquippedWeapon)
		{
			EquippedWeapon->SetShooterPawn(SKGShooterPawn);
		}
	}
}

void AShooterCombatCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterCombatCharacter, bIsDead);
}
