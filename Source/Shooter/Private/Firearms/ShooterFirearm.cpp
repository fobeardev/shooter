#include "Firearms/ShooterFirearm.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SKGFirearmComponent.h"
#include "Components/SKGAttachmentManagerComponent.h"
#include "Components/SKGProceduralAnimComponent.h"
#include "Components/SKGOffhandIKComponent.h"
//#include "Components/SKGMuzzleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

AShooterFirearm::AShooterFirearm()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);

    // Skeletal mesh root
    FirearmMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("FirearmMeshComponent");
    RootComponent = FirearmMeshComponent;

    // SKG components
    FirearmComponent = CreateDefaultSubobject<USKGFirearmComponent>("FirearmComponent");
    AttachmentManagerComponent = CreateDefaultSubobject<USKGAttachmentManagerComponent>("AttachmentManagerComponent");
    ProceduralAnimComponent = CreateDefaultSubobject<USKGProceduralAnimComponent>("ProceduralAnimComponent");
    //MuzzleComponent = CreateDefaultSubobject<USKGMuzzleComponent>("MuzzleComponent");
    OffhandIKComponent = CreateDefaultSubobject<USKGOffhandIKComponent>("OffhandIKComponent");

    if (FirearmComponent)
    {
        FirearmComponent->bAutoInitialize = false; // we will initialize manually
		FirearmComponent->SetFirearmMeshComponentName(FName("FirearmMeshComponent"));
		FirearmComponent->SetAttachmentManagerComponentName(FName("AttachmentManagerComponent"));
    }

    if (ProceduralAnimComponent)
    {
        ProceduralAnimComponent->bAutoInitialize = true;
        ProceduralAnimComponent->bOverrideComponentNames = false;
    }

    //if (MuzzleComponent)
    //{
    //    MuzzleComponent->bAutoInitialize = false;
    //    MuzzleComponent->bOverrideComponentNames = true;
    //}

    if (OffhandIKComponent)
    {
        OffhandIKComponent->bAutoInitialize = false;
        OffhandIKComponent->bOverrideComponentNames = true;
    }
}

void AShooterFirearm::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("ShooterFirearm: BeginPlay called for %s"), *GetName());

    // Ensure components exist before wiring
    if (!FirearmComponent || !FirearmMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ShooterFirearm] Missing FirearmComponent or Mesh on %s"), *GetName());
        return;
    }

    // Match SKGFirearm constructor behavior — assign names so SetupComponents() can resolve them
    const FName MeshName = FirearmMeshComponent->GetFName();
    FirearmComponent->SetFirearmMeshComponentName(MeshName);
    FirearmComponent->SetAttachmentManagerComponentName(
        AttachmentManagerComponent ? AttachmentManagerComponent->GetFName() : NAME_None
    );

    // Now initialize the firearm logic (handles internal component discovery)
    FirearmComponent->InitializeFirearmComponent();

    // Debug verification
    UE_LOG(LogTemp, Log, TEXT("[ShooterFirearm] Initialized firearm: %s"), *GetName());
    if (USKGProceduralAnimComponent* Proc = FirearmComponent->GetCurrentProceduralAnimComponent())
    {
        UE_LOG(LogTemp, Log, TEXT("[ShooterFirearm] CurrentProceduralAnimComponent = %s"), *GetNameSafe(Proc));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[ShooterFirearm] No CurrentProceduralAnimComponent found on %s"), *GetName());
    }

    // Disable tick on mesh (handled procedurally by SKG)
    if (FirearmMeshComponent)
    {
        FirearmMeshComponent->SetComponentTickEnabled(false);
    }

    // Validate core component references
    LogComponentInitialization();
}

void AShooterFirearm::LogComponentInitialization() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== AShooterFirearm Initialization ==="));
    UE_LOG(LogTemp, Warning, TEXT("Owner: %s"), *GetName());

    // Check each core component
    UE_LOG(LogTemp, Warning, TEXT("FirearmMeshComponent: %s"), FirearmMeshComponent ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("FirearmComponent: %s"), FirearmComponent ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("AttachmentManagerComponent: %s"), AttachmentManagerComponent ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("ProceduralAnimComponent: %s"), ProceduralAnimComponent ? TEXT("Valid") : TEXT("NULL"));
    //UE_LOG(LogTemp, Warning, TEXT("MuzzleComponent: %s"), MuzzleComponent ? TEXT("Valid") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("OffhandIKComponent: %s"), OffhandIKComponent ? TEXT("Valid") : TEXT("NULL"));

    UE_LOG(LogTemp, Warning, TEXT("FirearmMesh Forward: %s"),
        *FirearmMeshComponent->GetForwardVector().ToString());

    // If procedural animation exists, confirm its setup
    if (ProceduralAnimComponent)
    {
        UMeshComponent* Mesh = ProceduralAnimComponent->GetProceduralAnimMesh();
        const bool bHasMesh = Mesh != nullptr;

        UE_LOG(LogTemp, Warning, TEXT("ProceduralAnimComponent.Mesh: %s"), bHasMesh ? *Mesh->GetName() : TEXT("NULL"));

        // Aim sockets
        UE_LOG(LogTemp, Warning, TEXT("ProceduralAnimComponent AimSocketCount: %d"), ProceduralAnimComponent->ProceduralAimSocketNames.Num());
        for (const FName& SocketName : ProceduralAnimComponent->ProceduralAimSocketNames)
        {
            UE_LOG(LogTemp, Warning, TEXT("   AimSocket: %s"), *SocketName.ToString());
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("======================================"));
}

void AShooterFirearm::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
