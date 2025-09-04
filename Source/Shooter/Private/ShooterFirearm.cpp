#include "ShooterFirearm.h"
#include "Components/SKGShooterPawnComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "ShooterPDAFirearm.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

AShooterFirearm::AShooterFirearm()
{
    bReplicates = true;
    SetReplicateMovement(true);
}

void AShooterFirearm::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AShooterFirearm, ReloadCounter);
    DOREPLIFETIME(AShooterFirearm, bHasOwner);
}

void AShooterFirearm::BeginPlay()
{
    Super::BeginPlay();

    StartingTransform = GetTransform();

    if (FirearmMeshComponent)
    {
        FirearmMeshComponent->SetComponentTickEnabled(false);
    }

    if (DAConstruction)
    {
        OnDAConstructionBundlesLoaded();
    }
}

void AShooterFirearm::OnDAConstructionBundlesLoaded()
{
    if (const UShooterPDAFirearm* PDA = Cast<UShooterPDAFirearm>(DAConstruction))
    {
        SetFireRateDelay(PDA->FireRate);
        SetupFireModes(PDA->FireModes);

        if (PDA->AnimBP && FirearmMeshComponent)
        {
            FirearmMeshComponent->SetAnimInstanceClass(PDA->AnimBP);
        }
    }
}

void AShooterFirearm::SetOwningPawn(APawn* OwningPawn)
{
    SetOwner(OwningPawn);
    CurrentShooterPawnComponent = USKGShooterPawnComponent::GetShooterPawnComponent(OwningPawn);
    bHasOwner = UKismetSystemLibrary::IsValid(OwningPawn);
    OnRep_HasOwner();
}

void AShooterFirearm::Interact(APawn* InteractingPawn)
{
    SetReplicateMovement(false);
}

void AShooterFirearm::Drop()
{
    SetOwner(nullptr);
    bHasOwner = false;
    OnRep_HasOwner();

    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    FHitResult Dummy;
    K2_SetActorTransform(StartingTransform, false, Dummy, false);

    SetReplicateMovement(true);
}

void AShooterFirearm::SetupFireModes(const FGameplayTagContainer& FireModeTags)
{
    FireModes = FireModeTags;
    if (FireModes.Num() > 0)
    {
        CurrentFireMode = FireModes.GetByIndex(0);
    }
}

void AShooterFirearm::SetFireRateDelay(double FireRate)
{
    FireRateDelay = (FireRate > 0.0) ? (1.0f / static_cast<float>(FireRate)) : 0.0f;
}

void AShooterFirearm::OnRep_HasOwner()
{
}

void AShooterFirearm::PressTrigger()
{
    if (HasAuthority())
    {
        Server_PressTrigger();
    }
}

void AShooterFirearm::ReleaseTrigger()
{
    if (HasAuthority())
    {
        Server_ReleaseTrigger();
    }
}

void AShooterFirearm::Server_PressTrigger_Implementation()
{
    IncreasePressTrigger();
    Fire();
}

void AShooterFirearm::Server_ReleaseTrigger_Implementation()
{
    IncreaseReleaseTrigger();
    StopFire();
}

void AShooterFirearm::IncreasePressTrigger()
{
    // Placeholder for trigger press counter logic
}

void AShooterFirearm::IncreaseReleaseTrigger()
{
    // Placeholder for trigger release counter logic
}

bool AShooterFirearm::CanPerformAction() const
{
    return !bIsReloading && bHasOwner;
}

void AShooterFirearm::Fire()
{
    if (!CanPerformAction())
    {
        return;
    }

    PlayFireEffects();
    PerformFireAnimation();

    if (FirearmComponent)
    {
        LaunchProjectile(FirearmComponent->GetMuzzleProjectileTransform(100.f, 1.f), true);
    }

    if (FireModes.HasTag(FGameplayTag::RequestGameplayTag(FName("Firearm.FireMode.Auto"))))
    {
        GetWorldTimerManager().ClearTimer(FullAutoTimerHandle);
        GetWorldTimerManager().SetTimer(FullAutoTimerHandle, this, &AShooterFirearm::Fire, FireRateDelay, true);
    }
}

void AShooterFirearm::StopFire()
{
    GetWorldTimerManager().ClearTimer(FullAutoTimerHandle);
}

void AShooterFirearm::PerformFireAnimation()
{
    if (FirearmMeshComponent && FirearmMeshComponent->GetAnimInstance())
    {
        // Play montage here if you have one
    }
}

bool AShooterFirearm::IsDamagingProjectile(int32 ProjectileId) const
{
    return ProjectileId == DamagingProjectileId;
}

void AShooterFirearm::Server_LaunchProjectile_Implementation(const FSKGMuzzleTransform& LaunchTransform)
{
    LaunchProjectile(LaunchTransform, false);
}
