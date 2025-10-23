#include "Weapons/ShooterWeaponBase.h"
#include "Net/UnrealNetwork.h"
#include "Tags/ShooterGameplayTags.h"
#include "Components/SkeletalMeshComponent.h"

AShooterWeaponBase::AShooterWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Sensible defaults for tags (can be overridden per-asset/child)
	WeaponClassTag = ShooterTags::Weapon_Class_AR;				// arbitrary default
	DamageTypeTag = ShooterTags::Damage_Ballistic;				// default damage channel
	CurrentFireModeTag = ShooterTags::Weapon_FireMode_Semi;		// default fire mode

    WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMeshComponent;

    WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMeshComponent->SetGenerateOverlapEvents(false);
    WeaponMeshComponent->SetIsReplicated(true);
}

void AShooterWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AShooterWeaponBase::HandleFire_Internal() {}
void AShooterWeaponBase::HandleStopFire_Internal() {}

void AShooterWeaponBase::Fire()
{
    if (HasAuthority())
    {
        HandleFire_Internal();
    }
    else
    {
        Server_Fire();
    }
}

void AShooterWeaponBase::Server_Fire_Implementation()
{
    HandleFire_Internal();
}

void AShooterWeaponBase::StopFire()
{
    if (HasAuthority())
    {
        HandleStopFire_Internal();
    }
    else
    {
        Server_StopFire();
    }
}

void AShooterWeaponBase::Server_StopFire_Implementation()
{
    HandleStopFire_Internal();
}

bool AShooterWeaponBase::CanPerformAction() const
{
	// Extend this in child if you add cooldowns/overheats/etc.
	return !bIsReloading;
}

void AShooterWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params; Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(AShooterWeaponBase, CurrentFireModeTag, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(AShooterWeaponBase, bIsReloading, Params);
}
