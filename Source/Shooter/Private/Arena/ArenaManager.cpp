#include "Arena/ArenaManager.h"

#include "Characters/AI/ShooterAICharacter.h"
#include "Characters/ShooterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Tags/ShooterGameplayTags.h"
#include "Engine/World.h"
#include "Augments/AugmentPedestal.h"
#include "EngineUtils.h"
#include "Run/RunArenaData.h"

AArenaManager::AArenaManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AArenaManager::BeginPlay()
{
    Super::BeginPlay();

    PlayerRef = Cast<AShooterCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
    if (!PlayerRef)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArenaManager: Could not find player pawn at BeginPlay."));
    }
}

void AArenaManager::SetCurrentArenaData(URunArenaData* InData)
{
    CurrentArenaData = InData;
}

void AArenaManager::StartArena()
{
    if (Waves.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArenaManager: No waves configured on %s"), *GetName());
        return;
    }

    CurrentWaveIndex = 0;
    SpawnWave(CurrentWaveIndex);

    // Broadcast Event.Arena.Start
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        PlayerRef,
        ShooterTags::Event_Arena_Start,
        FGameplayEventData()
    );
}

void AArenaManager::SpawnWave(int32 WaveIndex)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("ArenaManager::SpawnWave: World is null."));
        return;
    }

    if (!Waves.IsValidIndex(WaveIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("ArenaManager::SpawnWave: Invalid wave index %d on %s"), WaveIndex, *GetName());
        return;
    }

    ActiveEnemies.Empty();

    const FArenaWaveData& Wave = Waves[WaveIndex];

    for (TSubclassOf<AShooterAICharacter> EnemyClass : Wave.EnemyClasses)
    {
        if (!EnemyClass)
        {
            continue;
        }

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        FVector SpawnLoc = GetActorLocation()
            + FVector(FMath::RandRange(-400.f, 400.f), FMath::RandRange(-400.f, 400.f), 0.f);
        FRotator SpawnRot = FRotator::ZeroRotator;

        AShooterAICharacter* Enemy = GetWorld()->SpawnActor<AShooterAICharacter>(
            EnemyClass,
            SpawnLoc,
            SpawnRot,
            Params
        );

        if (Enemy)
        {
            ActiveEnemies.Add(Enemy);
            Enemy->OnDestroyed.AddDynamic(this, &AArenaManager::OnEnemyDied);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ArenaManager: Spawned wave %d on %s (EnemyCount=%d)"),
        WaveIndex, *GetName(), ActiveEnemies.Num());
}

void AArenaManager::OnEnemyDied(AActor* DeadActor)
{
    AShooterAICharacter* DeadEnemy = Cast<AShooterAICharacter>(DeadActor);
    if (!DeadEnemy)
    {
        return;
    }

    ActiveEnemies.Remove(DeadEnemy);

    if (ActiveEnemies.Num() == 0)
    {
        HandleWaveCleared();
    }
}

void AArenaManager::HandleWaveCleared()
{
    // Broadcast Event.Arena.WaveCleared
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        PlayerRef,
        ShooterTags::Event_Arena_WaveCleared,
        FGameplayEventData()
    );

    // Let Blueprint do any fancy morphing between waves
    MorphArenaVisuals();

    const bool bLastWave = (CurrentWaveIndex == Waves.Num() - 1);
    const bool bArenaIsFinal = (CurrentArenaData && CurrentArenaData->bIsFinalArena);
    
    if (bLastWave)
    {
        if (bArenaIsFinal)
        {
            HandleArenaVictory(); // "You Won" -> back to hub
        }
        else
        {
            SpawnArenaPedestals(); // reward choice, Hades-style
        }
    }
    else
    {
        // Not last wave, just keep going
        CurrentWaveIndex++;
        SpawnWave(CurrentWaveIndex);
    }
}

void AArenaManager::HandleArenaEnd()
{
    UE_LOG(LogTemp, Warning, TEXT("ArenaManager: HandleArenaEnd fired. Sending Event_Run_ArenaCleared."));

    // Broadcast Event.Arena.End
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        PlayerRef,
        ShooterTags::Event_Arena_End,
        FGameplayEventData()
    );

    // Also notify RunDirector that this arena is cleared
    FGameplayEventData RunEventData;
    RunEventData.EventTag = ShooterTags::Event_Run_ArenaCleared;
    RunEventData.Instigator = this;
    RunEventData.Target = PlayerRef;

    UE_LOG(LogTemp, Warning, TEXT("ArenaManager: Sending Event_Run_ArenaCleared to %s"),
        *GetNameSafe(PlayerRef));

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        PlayerRef,
        ShooterTags::Event_Run_ArenaCleared,
        RunEventData
    );

    UE_LOG(LogTemp, Log, TEXT("ArenaManager: Arena complete on %s, notified RunDirector."), *GetName());
}

void AArenaManager::HandleArenaVictory()
{
    UE_LOG(LogTemp, Warning, TEXT("ArenaManager: Final arena cleared! Showing victory UI."));

    // Fire final arena event
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        PlayerRef,
        ShooterTags::Event_Arena_End,
        FGameplayEventData()
    );

    // Show "YOU WON" for 5 seconds
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            5.f,
            FColor::Yellow,
            TEXT("YOU WON!")
        );
    }

    // Delay before returning to hub
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        [this]()
        {
            // Notify RunDirector
            FGameplayEventData RunEventData;
            RunEventData.EventTag = ShooterTags::Event_Run_ArenaCleared;
            RunEventData.Instigator = this;
            RunEventData.Target = PlayerRef;

            UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
                PlayerRef,
                ShooterTags::Event_Run_ArenaCleared,
                RunEventData
            );

        },
        5.0f, // 5 seconds
        false
    );
}

void AArenaManager::OnAugmentChosen(AAugmentPedestal* PedestalChosen)
{
    // Destroy all pedestals
    for (TActorIterator<AAugmentPedestal> It(GetWorld()); It; ++It)
    {
        It->Destroy();
    }

    const bool bLastWave = (CurrentWaveIndex == Waves.Num() - 1);

    if (!bLastWave)
    {
        // This should never happen anymore — but safe check
        CurrentWaveIndex++;
        SpawnWave(CurrentWaveIndex);
        return;
    }

    // Last wave of the last wave index = End of arena
    HandleArenaEnd();
}

void AArenaManager::SpawnArenaPedestals()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("ArenaManager::SpawnArenaPedestals: World is null."));
        return;
    }

    if (!AugmentPedestalClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArenaManager::SpawnArenaPedestals: AugmentPedestalClass is null on %s"), *GetName());
        return;
    }

    FVector BaseLocation = GetActorLocation();
    float Spacing = 300.f;

    for (int32 i = 0; i < 3; i++)
    {
        FVector SpawnLoc = BaseLocation + FVector((i - 1) * Spacing, 0.f, 0.f);
        FRotator SpawnRot = FRotator::ZeroRotator;

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AAugmentPedestal* Pedestal = World->SpawnActor<AAugmentPedestal>(
            AugmentPedestalClass,
            SpawnLoc,
            SpawnRot,
            Params
        );

        if (Pedestal)
        {
            Pedestal->OnAugmentChosen.AddDynamic(this, &AArenaManager::OnAugmentChosen);
        }
    }

    // Broadcast Event.Arena.RewardSpawned
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        PlayerRef,
        ShooterTags::Event_Arena_RewardSpawned,
        FGameplayEventData()
    );

    UE_LOG(LogTemp, Log, TEXT("ArenaManager: Spawned augment pedestals on %s"), *GetName());
}
