#include "Gameplay/Combat/Projectile/ShooterProjectile.h"
#include "Gameplay/Combat/Projectile/ShooterProjectilePoolSubsystem.h"

#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

AShooterProjectile::AShooterProjectile()
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;
    bAlwaysRelevant = false;
    SetNetUpdateFrequency(60.0f);

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    SetRootComponent(CollisionComponent);
    CollisionComponent->InitSphereRadius(2.0f);
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    CollisionComponent->SetGenerateOverlapEvents(false);

    TracerComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Tracer"));
    TracerComponent->SetupAttachment(RootComponent);
    TracerComponent->bAutoActivate = false;

    Velocity = FVector::ZeroVector;
    ElapsedLifeTime = 0.0f;
    bIsActive = false;

    SetActorEnableCollision(false);
    SetActorHiddenInGame(true);
}

void AShooterProjectile::BeginPlay()
{
    Super::BeginPlay();
}

void AShooterProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AShooterProjectile, Velocity);
    DOREPLIFETIME(AShooterProjectile, StartLocation);
    DOREPLIFETIME(AShooterProjectile, ElapsedLifeTime);
    DOREPLIFETIME(AShooterProjectile, bIsActive);
}

void AShooterProjectile::InitializeFromSpec(
    const FVector& SpawnLocation,
    const FVector& ShootDirection,
    const FProjectileConfig& InConfig,
    const FProjectileIdentity& InIdentity,
    AActor* InInstigatorActor,
    AController* InInstigatorController,
    UShooterProjectilePoolSubsystem* InOwningPool)
{
    SetActorLocation(SpawnLocation);
    SetActorRotation(ShootDirection.Rotation());

    Config = InConfig;
    Identity = InIdentity;

    InstigatorActorWeak = InInstigatorActor;
    InstigatorControllerWeak = InInstigatorController;
    OwningPool = InOwningPool;

    RemainingRicochets = FMath::Max(0, Config.MaxRicochets);
    RemainingPierces = FMath::Max(0, Config.MaxPierces);

    // Initialize velocity from direction + speed
    Velocity = ShootDirection.GetSafeNormal() * Config.InitialSpeed;

    // Lifetime
    LifeTimeRemaining = FMath::Max(0.01f, Config.MaxLifeTime);

    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);

    // Optional: tracer FX spawn if you do that in projectile
    if (Config.bSpawnTracerFX)
    {
        SpawnTracerFX();
    }
}


void AShooterProjectile::OnPooledActivate(
    const FVector& SpawnLocation,
    const FVector& Direction,
    AController* InstigatorController,
    AActor* InstigatorActor
)
{
    bIsActive = true;
    ElapsedLifeTime = 0.0f;
    StartLocation = SpawnLocation;

    SetActorLocation(SpawnLocation);
    Velocity = Direction.GetSafeNormal() * Config.InitialSpeed;

    InstigatorControllerWeak = InstigatorController;
    InstigatorActorWeak = InstigatorActor;

    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);

    if (Config.bSpawnTracerFX && TracerComponent)
    {
        TracerComponent->Activate(true);
    }
}

void AShooterProjectile::OnPooledDeactivate()
{
    bIsActive = false;

    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    if (TracerComponent)
    {
        TracerComponent->Deactivate();
    }

    Velocity = FVector::ZeroVector;
    ElapsedLifeTime = 0.0f;

    if (OwningPool.IsValid())
    {
        OwningPool->ReturnToPool(this);
    }
}

void AShooterProjectile::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Server drives movement; clients just receive replication.
    if (!bIsActive || !HasAuthority())
    {
        return;
    }

    ElapsedLifeTime += DeltaSeconds;
    if (ElapsedLifeTime >= Config.MaxLifeTime)
    {
        OnPooledDeactivate();
        return;
    }

    if (!FMath::IsNearlyZero(Config.GravityScale))
    {
        const float GravityZ = GetWorld()->GetGravityZ();
        Velocity += FVector(0.0f, 0.0f, GravityZ * Config.GravityScale * DeltaSeconds);
    }

    const FVector CurrentLocation = GetActorLocation();
    const FVector Delta = Velocity * DeltaSeconds;
    const FVector TargetLocation = CurrentLocation + Delta;

    FHitResult HitResult;
    bool bHit = false;

    FCollisionQueryParams QueryParams(TEXT("ProjectileTrace"), true, InstigatorActorWeak.Get());
    QueryParams.bReturnPhysicalMaterial = false;

    if (Config.bUseSphereTrace)
    {
        bHit = GetWorld()->SweepSingleByChannel(
            HitResult,
            CurrentLocation,
            TargetLocation,
            FQuat::Identity,
            Config.CollisionChannel,
            FCollisionShape::MakeSphere(Config.Radius),
            QueryParams
        );
    }
    else
    {
        bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            CurrentLocation,
            TargetLocation,
            Config.CollisionChannel,
            QueryParams
        );
    }

    if (bHit)
    {
        SetActorLocation(HitResult.Location);
        HandleProjectileHit(HitResult);
    }
    else
    {
        SetActorLocation(TargetLocation);
    }

    UpdateTracerFX();
}

void AShooterProjectile::HandleProjectileHit(const FHitResult& Hit)
{
    if (!HasAuthority())
    {
        return;
    }

    ApplyDamage(Hit);
    SpawnImpactFX(Hit);

    OnPooledDeactivate();
}

void AShooterProjectile::ApplyDamage(const FHitResult& Hit)
{
    AActor* HitActor = Hit.GetActor();
    if (!HitActor)
    {
        return;
    }

    if (Config.DamageGameplayEffectClass.ToSoftObjectPath().IsValid())
    {
        UAbilitySystemComponent* SourceASC =
            UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InstigatorActorWeak.Get());

        UAbilitySystemComponent* TargetASC =
            UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor);

        if (!SourceASC || !TargetASC)
        {
            return;
        }

        TSubclassOf<UGameplayEffect> DamageGE =
            Config.DamageGameplayEffectClass.LoadSynchronous();

        if (!DamageGE)
        {
            return;
        }

        // Convert cm/s -> m/s
        const float VelocityMS = Velocity.Size() / 100.0f;

        // Guard against invalid mass
        const float BulletMassKg = FMath::Max(Config.BulletMassKg, 0.001f);

        // KE = 0.5 * m * v^2
        const float KE = 0.5f * BulletMassKg * (VelocityMS * VelocityMS);

        // Map to gameplay damage window
        const float ImpactEnergy = FMath::GetMappedRangeValueClamped(
            FVector2D(0.0f, 3500.0f),
            FVector2D(10.0f, 50.0f),
            KE
        );

        FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
        Context.AddInstigator(InstigatorActorWeak.Get(), this);

        FGameplayEffectSpecHandle SpecHandle =
            SourceASC->MakeOutgoingSpec(DamageGE, 1.0f, Context);

        if (!SpecHandle.IsValid())
        {
            return;
        }

        SpecHandle.Data->SetSetByCallerMagnitude(
            Config.SetByCallerDamageTag,
            -ImpactEnergy
        );

        TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        return;
    }

    // Fallback non-GAS damage
    if (Config.Damage > 0.0f)
    {
        TSubclassOf<UDamageType> DamageClass = Config.DamageTypeClass;
        if (!DamageClass)
        {
            DamageClass = UDamageType::StaticClass();
        }

        UGameplayStatics::ApplyPointDamage(
            HitActor,
            Config.Damage,
            Velocity.GetSafeNormal(),
            Hit,
            InstigatorControllerWeak.Get(),
            InstigatorActorWeak.Get(),
            DamageClass
        );
    }
}

void AShooterProjectile::SpawnImpactFX(const FHitResult& Hit)
{
    if (!Config.ImpactFX.IsValid())
    {
        return;
    }

    UNiagaraSystem* ImpactSystem = Config.ImpactFX.Get();
    if (!ImpactSystem)
    {
        return;
    }

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        ImpactSystem,
        Hit.ImpactPoint,
        Hit.ImpactNormal.Rotation()
    );
}

void AShooterProjectile::UpdateTracerFX()
{
    if (!TracerComponent || !Config.bSpawnTracerFX)
    {
        return;
    }

    // Example of parameter update (optional)
    // const float Distance = FVector::Dist(StartLocation, GetActorLocation());
    // TracerComponent->SetFloatParameter(TEXT("Distance"), Distance);
}
