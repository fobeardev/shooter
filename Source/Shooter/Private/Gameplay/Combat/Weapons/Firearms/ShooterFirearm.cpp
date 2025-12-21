#include "Gameplay/Combat/Weapons/Firearms/ShooterFirearm.h"

// Engine headers
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"

// Project headers
#include "Gameplay/Characters/ShooterCombatCharacter.h"
#include "Gameplay/Combat/Projectile/ShooterProjectilePoolSubsystem.h"
#include "Gameplay/Combat/Projectile/ProjectileConfig.h"
#include "Gameplay/Tags/ShooterGameplayTags.h"

// SKG headers
#include "Components/SKGFirearmComponent.h"
#include "Components/SKGAttachmentManagerComponent.h"
#include "Components/SKGProceduralAnimComponent.h"
#include "Components/SKGOffhandIKComponent.h"
#include "Components/SKGMuzzleComponent.h"
#include "Components/SKGShooterPawnComponent.h"

// Constructor
AShooterFirearm::AShooterFirearm()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);

    FirearmComponent = CreateDefaultSubobject<USKGFirearmComponent>(TEXT("FirearmComponent"));
    AttachmentManagerComponent = CreateDefaultSubobject<USKGAttachmentManagerComponent>(TEXT("AttachmentManagerComponent"));
    MuzzleComponent = CreateDefaultSubobject<USKGMuzzleComponent>(TEXT("MuzzleComponent"));
    OffhandIKComponent = CreateDefaultSubobject<USKGOffhandIKComponent>(TEXT("OffhandIKComponent"));

    WeaponClassTag = ShooterTags::Weapon_Class_AR;
    DamageTypeTag = ShooterTags::Damage_Ballistic;
    CurrentFireModeTag = ShooterTags::Weapon_FireMode_Semi;

    WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMeshComponent;

    if (WeaponMeshComponent)
    {
        WeaponMeshComponent->SetCollisionProfileName(TEXT("Holdable"));
        WeaponMeshComponent->SetIsReplicated(true);
    }
}

// Overrides
void AShooterFirearm::BeginPlay()
{
    Super::BeginPlay();

    if (WeaponMeshComponent)
    {
        WeaponMeshComponent->SetComponentTickEnabled(false);
    }

    if (ProceduralAnimComponent)
    {
        ProceduralAnimComponent->SetProceduralMeshName(WeaponMeshComponent->GetFName());
        ProceduralAnimComponent->InitializeProceduralAnimComponent();
    }
}

void AShooterFirearm::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

// Public methods
bool AShooterFirearm::CanPerformAction() const
{
    return Super::CanPerformAction();
}

void AShooterFirearm::SetFireConfig(const FProjectileConfig& InConfig)
{
    FireConfig = InConfig;
}

FSKGMuzzleTransform AShooterFirearm::GetMuzzleTransform() const
{
    if (FirearmComponent)
    {
        return FirearmComponent->GetMuzzleProjectileTransform(100.f, 1.f);
    }

    return FSKGMuzzleTransform();
}

void AShooterFirearm::Fire()
{
    if (!CanPerformAction())
    {
        return;
    }

    ++PressCount;

    // Server authoritative fire flow (unchanged)
    Server_Fire();
}

void AShooterFirearm::StopFire()
{
    ClearFireTimers();
    ++ReleaseCount;
}

void AShooterFirearm::FireWithProjectileSpec(const FProjectileConfig& Config, const FProjectileIdentity& Identity)
{
    if (!CanPerformAction())
    {
        return;
    }

    // Ability may call this on client; server must be authoritative for spawn
    if (HasAuthority())
    {
        Server_FireWithProjectileSpec(Config, Identity);
    }
    else
    {
        Server_FireWithProjectileSpec(Config, Identity);
    }

    // Cosmetic client feedback stays local
    PlayFireEffects();

    if (ShooterPawn)
    {
        ShooterPawn->PerformProceduralRecoil(FRotator(1.f), FVector(1.f), FRotator(1.f));
    }
}

// Protected methods
void AShooterFirearm::Server_Fire_Implementation()
{
    HandleFire_Internal();
    BeginAutoIfNeeded();
}


void AShooterFirearm::Server_FireWithProjectileSpec_Implementation(
    const FProjectileConfig& Config,
    const FProjectileIdentity& Identity)
{
    if (!GetWorld() || !FirearmComponent)
    {
        return;
    }

    // Get muzzle transform from SKG (this keeps S_Muzzle correctness)
    const FSKGMuzzleTransform MuzzleXform =
        FirearmComponent->GetMuzzleProjectileTransform(100.f, 1.f);

    const FTransform Xf = MuzzleXform.ConvertToTransform();
    const FVector FireLoc = Xf.GetLocation();
    const FVector FireDir = Xf.GetRotation().GetForwardVector();

    UShooterProjectilePoolSubsystem* Pool =
        GetWorld()->GetSubsystem<UShooterProjectilePoolSubsystem>();

    if (!Pool)
    {
        return;
    }

    // Server spawns authoritative projectile using pool’s internal class selection
    Pool->SpawnProjectile(
        FireLoc,
        FireDir,
        Config,
        Identity,
        GetOwner(),
        GetInstigatorController()
    );

    // bookkeeping + FX (server-safe; clients can still do predicted cosmetics separately)
    if (FirearmComponent)
    {
        FirearmComponent->ShotPerformed();
    }

    PlayFireEffects();

    if (ShooterPawn)
    {
        ShooterPawn->PerformProceduralRecoil(FRotator(1.f), FVector(1.f), FRotator(1.f));
    }
}

// Private methods
void AShooterFirearm::HandleFire_Internal()
{
    UE_LOG(LogTemp, Warning, TEXT("[Firearm] HandleFire_Internal called on %s"), *GetNameSafe(this));

    if (!FirearmComponent)
    {
        return;
    }

    const FSKGMuzzleTransform MuzzleXform = FirearmComponent->GetMuzzleProjectileTransform(100.f, 1.f);

    if (HasAuthority())
    {
        LaunchProjectile(MuzzleXform, FireConfig, false);
    }

    FirearmComponent->ShotPerformed();

    PlayFireEffects();

    if (ShooterPawn)
    {
        ShooterPawn->PerformProceduralRecoil(FRotator(1.f), FVector(1.f), FRotator(1.f));
    }
}

void AShooterFirearm::HandleStopFire_Internal()
{
    if (GetWorldTimerManager().IsTimerActive(AutoTimerHandle))
    {
        GetWorldTimerManager().ClearTimer(AutoTimerHandle);
    }
}

void AShooterFirearm::BeginAutoIfNeeded()
{
    if (CurrentFireModeTag == ShooterTags::Weapon_FireMode_Auto)
    {
        if (!GetWorldTimerManager().IsTimerActive(AutoTimerHandle))
        {
            GetWorldTimerManager().SetTimer(
                AutoTimerHandle,
                this,
                &AShooterFirearm::HandleFire_Internal,
                FMath::Max(0.01f, FireRateSeconds),
                true
            );
        }
    }
    else if (CurrentFireModeTag == ShooterTags::Weapon_FireMode_Burst)
    {
        PendingBurstShots = BurstSize - 1;
        if (PendingBurstShots > 0)
        {
            GetWorldTimerManager().SetTimer(
                AutoTimerHandle,
                [this]()
                {
                    if (PendingBurstShots <= 0)
                    {
                        ClearFireTimers();
                        return;
                    }

                    HandleFire_Internal();
                    --PendingBurstShots;
                },
                FMath::Max(0.01f, FireRateSeconds),
                true
            );
        }
    }
}

void AShooterFirearm::ClearFireTimers()
{
    GetWorldTimerManager().ClearTimer(AutoTimerHandle);
    PendingBurstShots = 0;
}
void AShooterFirearm::LaunchProjectile(
    const FSKGMuzzleTransform& LaunchTransform,
    const FProjectileConfig& Config,
    bool bIsLocalFire)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const FTransform MuzzleXf = LaunchTransform.ConvertToTransform();
    const FVector FireLocation = MuzzleXf.GetLocation();
    const FVector FireDirection = MuzzleXf.GetRotation().GetForwardVector();

    // ----------------------------------------
    // CLIENT: cosmetic only
    // ----------------------------------------
    if (World->GetNetMode() == NM_Client)
    {
        if (bIsLocalFire)
        {
            PlayFireEffects();
        }
        return;
    }

    // ----------------------------------------
    // SERVER: authoritative projectile spawn
    // ----------------------------------------
    UShooterProjectilePoolSubsystem* Pool =
        World->GetSubsystem<UShooterProjectilePoolSubsystem>();

    if (!Pool)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[ShooterFirearm] ProjectilePoolSubsystem missing on %s"),
            *GetNameSafe(this));
        return;
    }

    const int32 ProjectileCount = FMath::Max(1, Config.ProjectileCount);
    const float HalfAngleDeg = FMath::Max(0.0f, Config.SpreadHalfAngleDeg);
    const float HalfAngleRad = FMath::DegreesToRadians(HalfAngleDeg);

    for (int32 i = 0; i < ProjectileCount; ++i)
    {
        const FVector ShotDir =
            (ProjectileCount > 1 && HalfAngleRad > 0.0f)
                ? FMath::VRandCone(FireDirection, HalfAngleRad)
                : FireDirection;

        Pool->SpawnProjectile(
            FireLocation,
            ShotDir,
            Config,
            /*Identity*/ FProjectileIdentity::MakeWeaponIdentity(this),
            /*InstigatorActor*/ GetOwner(),
            /*InstigatorController*/ GetInstigatorController()
        );
    }

    // ----------------------------------------
    // Server-side weapon bookkeeping
    // ----------------------------------------
    FirearmComponent->ShotPerformed();

    if (bIsLocalFire)
    {
        PlayFireEffects();
    }
}
