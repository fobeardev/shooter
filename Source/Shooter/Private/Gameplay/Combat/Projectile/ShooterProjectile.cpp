#include "Gameplay/Combat/Projectile/ShooterProjectile.h"

#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

AShooterProjectile::AShooterProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    bAlwaysRelevant = false; // You can tweak relevancy later.
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

void AShooterProjectile::InitializeFromConfig(const FProjectileConfig& InConfig)
{
    Config = InConfig;
    CollisionComponent->SetSphereRadius(Config.Radius);

    if (Config.TracerFX.IsValid())
    {
        if (UNiagaraSystem* TracerSystem = Config.TracerFX.Get())
        {
            TracerComponent->SetAsset(TracerSystem);
        }
    }
}

void AShooterProjectile::OnPooledActivate(const FVector& SpawnLocation, const FVector& Direction, AController* InstigatorController, AActor* InstigatorActor)
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
}

void AShooterProjectile::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bIsActive || !HasAuthority())
    {
        // Server drives movement; clients receive replicated transforms.
        return;
    }

    ElapsedLifeTime += DeltaSeconds;
    if (ElapsedLifeTime >= Config.MaxLifeTime)
    {
        OnPooledDeactivate();
        return;
    }

    // Simple gravity
    if (!FMath::IsNearlyZero(Config.GravityScale))
    {
        Velocity += FVector(0.0f, 0.0f, GetWorld()->GetGravityZ() * Config.GravityScale * DeltaSeconds);
    }

    const FVector CurrentLocation = GetActorLocation();
    const FVector Delta = Velocity * DeltaSeconds;
    const FVector TargetLocation = CurrentLocation + Delta;

    FHitResult HitResult;
    bool bHit = false;

    if (Config.bUseSphereTrace)
    {
        bHit = GetWorld()->SweepSingleByChannel(
            HitResult,
            CurrentLocation,
            TargetLocation,
            FQuat::Identity,
            Config.CollisionChannel,
            FCollisionShape::MakeSphere(Config.Radius),
            FCollisionQueryParams(TEXT("ProjectileTrace"), true, InstigatorActorWeak.Get())
        );
    }
    else
    {
        bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            CurrentLocation,
            TargetLocation,
            Config.CollisionChannel,
            FCollisionQueryParams(TEXT("ProjectileTrace"), true, InstigatorActorWeak.Get())
        );
    }

    if (bHit)
    {
        SetActorLocation(HitResult.Location);
        OnProjectileHit(HitResult);
    }
    else
    {
        SetActorLocation(TargetLocation);
    }

    UpdateTracerFX();
}

void AShooterProjectile::OnProjectileHit(const FHitResult& Hit)
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

    AController* InstigatorController = InstigatorControllerWeak.Get();
    AActor* InstigatorActor = InstigatorActorWeak.Get();

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
            InstigatorController,
            InstigatorActor,
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

    if (UNiagaraSystem* ImpactSystem = Config.ImpactFX.Get())
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            ImpactSystem,
            Hit.ImpactPoint,
            Hit.ImpactNormal.Rotation()
        );
    }
}

void AShooterProjectile::UpdateTracerFX()
{
    if (!TracerComponent || !Config.bSpawnTracerFX)
    {
        return;
    }

    // Optional: update parameters like speed, distance from StartLocation, etc.
    // Example:
    // const float Distance = FVector::Dist(StartLocation, GetActorLocation());
    // TracerComponent->SetFloatParameter(TEXT("Distance"), Distance);
}
