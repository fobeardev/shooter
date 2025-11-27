#include "Run/RunDirector.h"
#include "Run/RunDirectorConfig.h"
#include "Run/RunArenaData.h"
#include "Arena/ArenaManager.h"
#include "Characters/ShooterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Tags/ShooterGameplayTags.h"
#include "EngineUtils.h"

URunDirector::URunDirector()
{
    ConfigAsset = TSoftObjectPtr<URunDirectorConfig>(
        FSoftObjectPath("/Game/Hadeslike/Core/DA_RunDirectorConfig.DA_RunDirectorConfig")
    );
}

void URunDirector::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (ConfigAsset.IsNull())
    {
        UE_LOG(LogTemp, Error, TEXT("RunDirector: ConfigAsset soft path is NULL."));
        return;
    }

    URunDirectorConfig* Loaded = ConfigAsset.LoadSynchronous();
    if (!Loaded)
    {
        UE_LOG(LogTemp, Error, TEXT("RunDirector: Failed to load RunDirectorConfig asset."));
        return;
    }

    ArenaSequence = Loaded->ArenaSequence;

    UE_LOG(LogTemp, Log, TEXT("RunDirector: Loaded %d arenas from config."), ArenaSequence.Num());

    // Listen for map loads
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
        this,
        &URunDirector::OnArenaLevelLoaded
    );
}

void URunDirector::Deinitialize()
{
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
}

void URunDirector::StartNewRun()
{
    BindRunEventsToCurrentASC();

    PlayerRef = Cast<AShooterCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

    if (!PlayerRef)
    {
        UE_LOG(LogTemp, Error, TEXT("RunDirector: Could not find player pawn."));
        return;
    }

    // Subscribe to arena-cleared event for this run
    if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerRef))
    {
        ASC->GenericGameplayEventCallbacks
            .FindOrAdd(ShooterTags::Event_Run_ArenaCleared)
            .AddUObject(this, &URunDirector::OnArenaCleared);
    }

    if (ArenaSequence.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("RunDirector: No arenas assigned."));
        return;
    }

    CurrentArenaIndex = 0;
    CurrentRunIndex++;
    CurrentDifficultyTier = 1;

    LoadArenaByIndex(CurrentArenaIndex);
}

void URunDirector::LoadArenaByIndex(int32 Index)
{
    if (!ArenaSequence.IsValidIndex(Index))
    {
        UE_LOG(LogTemp, Error, TEXT("RunDirector: Invalid arena index %d"), Index);
        return;
    }

    CurrentArenaData = ArenaSequence[Index];
    if (!CurrentArenaData)
    {
        UE_LOG(LogTemp, Error, TEXT("RunDirector: ArenaData is NULL at index %d"), Index);
        return;
    }

    if (CurrentArenaData->ArenaLevel.IsNull())
    {
        UE_LOG(LogTemp, Error, TEXT("RunDirector: ArenaLevel is NULL in DataAsset '%s'"),
            *CurrentArenaData->GetName());
        return;
    }

    // Fire Arena.Start
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        PlayerRef,
        ShooterTags::Event_Arena_Start,
        FGameplayEventData()
    );

    const FName LevelName(*CurrentArenaData->ArenaLevel.GetLongPackageName());

    UE_LOG(LogTemp, Log, TEXT("RunDirector: Loading Arena '%s' (Final=%d)"),
        *LevelName.ToString(),
        CurrentArenaData->bIsFinalArena ? 1 : 0
    );

    UGameplayStatics::OpenLevel(GetWorld(), LevelName);
}

void URunDirector::OnArenaCleared(const FGameplayEventData* Payload)
{
    if (!CurrentArenaData)
    {
        UE_LOG(LogTemp, Error, TEXT("RunDirector: OnArenaCleared but CurrentArenaData is NULL."));
        ReturnToHub();
        return;
    }

    // If THIS arena was marked final, end the run right now
    if (CurrentArenaData->bIsFinalArena)
    {
        UE_LOG(LogTemp, Log, TEXT("RunDirector: Final arena cleared. Returning to Hub."));
        ReturnToHub();
        return;
    }

    // Otherwise move to next arena in sequence
    CurrentArenaIndex++;

    if (ArenaSequence.IsValidIndex(CurrentArenaIndex))
    {
        LoadArenaByIndex(CurrentArenaIndex);
    }
    else
    {
        // End of sequence, even if not marked final (fallback)
        UE_LOG(LogTemp, Warning, TEXT("RunDirector: Sequence ended but arena was not marked final. Returning to Hub."));
        ReturnToHub();
    }
}

void URunDirector::OnArenaLevelLoaded(UWorld* LoadedWorld)
{
    UE_LOG(LogTemp, Error, TEXT("[DEBUG] OnArenaLevelLoaded fired for world: %s"),
        *LoadedWorld->GetName());

	BindRunEventsToCurrentASC();

    if (!LoadedWorld)
        return;

    UE_LOG(LogTemp, Log, TEXT("RunDirector: Arena level loaded. Searching for ArenaManager."));

    for (TActorIterator<AArenaManager> It(LoadedWorld); It; ++It)
    {
        AArenaManager* Manager = *It;

        // Pass current arena data in:
        if (ArenaSequence.IsValidIndex(CurrentArenaIndex))
        {
            Manager->SetCurrentArenaData(ArenaSequence[CurrentArenaIndex]);
        }

        Manager->StartArena();
        return;
    }

    UE_LOG(LogTemp, Error, TEXT("RunDirector: No ArenaManager found in loaded level."));
}

void URunDirector::ReturnToHub()
{
    UE_LOG(LogTemp, Log, TEXT("RunDirector: Opening Hub (L_Hub)."));

    UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("L_Hub")));
}

void URunDirector::HandlePlayerDeath()
{
    UE_LOG(LogTemp, Log, TEXT("RunDirector: Player died. Returning to Hub."));
    ReturnToHub();
}

void URunDirector::BindRunEventsToCurrentASC()
{
    PlayerRef = Cast<AShooterCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

    if (!PlayerRef)
    {
        UE_LOG(LogTemp, Error, TEXT("RunDirector: No PlayerRef in BindRunEventsToCurrentASC"));
        return;
    }

    UAbilitySystemComponent* ASC =
        UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerRef);

    if (!ASC)
    {
        UE_LOG(LogTemp, Error, TEXT("RunDirector: No ASC found on PlayerRef in BindRunEventsToCurrentASC"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("RunDirector: Binding Event_Run_ArenaCleared on ASC %s for pawn %s"),
        *GetNameSafe(ASC), *GetNameSafe(PlayerRef));

    ASC->GenericGameplayEventCallbacks
        .FindOrAdd(ShooterTags::Event_Run_ArenaCleared)
        .AddUObject(this, &URunDirector::OnArenaCleared);
}
