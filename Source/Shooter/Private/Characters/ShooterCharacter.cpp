#include "Characters/ShooterCharacter.h"

#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffect.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"

// If you keep your AttributeSet in a separate header:
#include "Abilities/AttrSet_Combat.h"

AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Create and configure ASC
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>("ASC");
	ASC->SetIsReplicated(true);
	// Mixed is a good default for characters (replicates GE data efficiently)
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Create AttributeSet (non-replicated UObject owned by ASC/Actor; attributes themselves replicate through ASC)
	CombatAttributes = CreateDefaultSubobject<UAttrSet_Combat>("AttrSet_Combat");

	// (Optional) Set default tags here if you prefer code over BP defaults:
	// Tag_Ability_Dash = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Dash"));
	// Tag_State_IFrame = FGameplayTag::RequestGameplayTag(FName("State.IFrame"));
}

UAbilitySystemComponent* AShooterCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// On server we can grant startup abilities here, but PossessedBy is a safer hook
	// after PlayerState exists. We still ensure ASC is initialized locally for owner-only logic.
	if (ASC && GetLocalRole() == ROLE_Authority)
	{
		InitializeASC();
		GrantStartupAbilities();
	}
}

void AShooterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Server: PlayerState is valid now; (re)initialize ASC & grant abilities
	if (ASC && GetLocalRole() == ROLE_Authority)
	{
		InitializeASC();
		GrantStartupAbilities();
	}
}

void AShooterCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Client: re-init ASC actor info after PlayerState replicates
	if (ASC)
	{
		ASC->InitAbilityActorInfo(this, this);
	}
}

void AShooterCharacter::InitializeASC()
{
	// Tie ASC to this pawn (Avatar) and its owner (PlayerState/Controller chain)
	ASC->InitAbilityActorInfo(this, this);

	// You can set prediction windows or input binding policies here if desired
	// ASC->SetTagRelationshipMapping(...);
}

void AShooterCharacter::GrantStartupAbilities()
{
	if (!ensure(GetLocalRole() == ROLE_Authority)) return;
	if (!ASC) return;

	const FGameplayTag StartupGivenTag = FGameplayTag::RequestGameplayTag(TEXT("Internal.Startup.Given"));

	// If we've already granted them, bail
	if (ASC->HasMatchingGameplayTag(StartupGivenTag))
	{
		return;
	}

	if (DashAbilityClass)
	{
		FGameplayAbilitySpec Spec(DashAbilityClass, /*Level*/1, /*InputID*/ INDEX_NONE, this);
		if (Tag_Ability_Dash.IsValid())
		{
			Spec.DynamicAbilityTags.AddTag(Tag_Ability_Dash);
		}
		ASC->GiveAbility(Spec);
	}

	// Mark as done (loose tag lives on ASC and replicates)
	ASC->AddLooseGameplayTag(StartupGivenTag);
	// Ensure actor info is initialized (safe to call again)
	ASC->InitAbilityActorInfo(this, this);
}

bool AShooterCharacter::IsInIFrame() const
{
	return ASC && Tag_State_IFrame.IsValid() && ASC->HasMatchingGameplayTag(Tag_State_IFrame);
}

void AShooterCharacter::Input_Dash()
{
	// Local request -> server RPC (authoritative)
	if (HasAuthority())
	{
		// Server can immediately activate by tag
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

void AShooterCharacter::ServerTryActivateDash_Implementation()
{
	if (ASC && Tag_Ability_Dash.IsValid())
	{
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(Tag_Ability_Dash));
	}
}

void AShooterCharacter::PauseFirearmsDuringDash(float PauseForSeconds)
{
	// Optional: if you want to stop firing briefly when dash starts, hook this from your Dash GA via BP or Interface.
	// e.g., call into your current AShooterFirearm and StopFire(); set timers to re-enable.
	FTimerHandle Dummy;
	GetWorldTimerManager().SetTimer(Dummy, []() { /* re-enable firing */ }, PauseForSeconds, false);
}

// Replication setup (Character has replicated movement by default; add more if you add custom vars)
void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Attribute values replicate through ASC; you typically don't replicate the AttributeSet pointer itself.
}
