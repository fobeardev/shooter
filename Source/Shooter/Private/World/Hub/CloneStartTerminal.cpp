#include "World/Hub/CloneStartTerminal.h"
#include "World/Hub/CloneFacilityManager.h"
#include "Gameplay/Characters/Player/ShooterCharacter.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "EngineUtils.h"

ACloneStartTerminal::ACloneStartTerminal()
{
    PrimaryActorTick.bCanEverTick = false;

    TerminalMesh = CreateDefaultSubobject<UStaticMeshComponent>("TerminalMesh");
    RootComponent = TerminalMesh;

    InteractionZone = CreateDefaultSubobject<UBoxComponent>("InteractionZone");
    InteractionZone->SetupAttachment(RootComponent);
    InteractionZone->SetBoxExtent(FVector(100.f, 100.f, 100.f));
    InteractionZone->SetCollisionProfileName("Trigger");

    InteractionPrompt = CreateDefaultSubobject<UWidgetComponent>("InteractionPrompt");
    InteractionPrompt->SetupAttachment(RootComponent);
}

void ACloneStartTerminal::BeginPlay()
{
    Super::BeginPlay();

    // find the facility manager in the level
    for (TActorIterator<ACloneFacilityManager> It(GetWorld()); It; ++It)
    {
        FacilityManagerRef = *It;
        break;
    }

    if (InteractionZone)
    {
        InteractionZone->OnComponentBeginOverlap.AddDynamic(
            this,
            &ACloneStartTerminal::OnInteract
        );
    }
}

void ACloneStartTerminal::OnInteract(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (bHasTriggeredRun)
        return;

    if (!OtherActor || !OtherActor->IsA(AShooterCharacter::StaticClass()))
        return;

    bHasTriggeredRun = true;

    UE_LOG(LogTemp, Log, TEXT("[CloneStartTerminal] Player overlapped — starting run."));
    BeginRun();
}

void ACloneStartTerminal::BeginRun()
{
    if (FacilityManagerRef)
    {
        UE_LOG(LogTemp, Log, TEXT("[CloneStartTerminal] Triggering FacilityManager->BeginRun()."));
        FacilityManagerRef->BeginRun();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[CloneStartTerminal] No FacilityManager found."));
    }
}
