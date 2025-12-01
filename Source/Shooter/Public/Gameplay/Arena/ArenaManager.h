#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ArenaManager.generated.h"

class AShooterAICharacter;
class AShooterCharacter;
class AAugmentPedestal;
class URunArenaData;

USTRUCT(BlueprintType)
struct FArenaWaveData
{
    GENERATED_BODY()

    // Enemy types to spawn in this wave
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Wave")
    TArray<TSubclassOf<AShooterAICharacter>> EnemyClasses;

    // (Optional) Delay before this wave starts (not yet used here, but useful later)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Wave")
    float SpawnDelay = 1.0f;

    // (Optional) Biome/theme tag for this wave (for music, VFX, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arena|Wave")
    FGameplayTag BiomeTag;
};

UCLASS()
class SHOOTER_API AArenaManager : public AActor
{
    GENERATED_BODY()

public:
    AArenaManager();

protected:
    virtual void BeginPlay() override;

    // Sequence of waves for this arena (room)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena")
    TArray<FArenaWaveData> Waves;

    // Enemies currently alive in the active wave
    UPROPERTY()
    TArray<AShooterAICharacter*> ActiveEnemies;

    // Index of the current wave (0-based). -1 means not started.
    UPROPERTY()
    int32 CurrentWaveIndex = -1;

    UPROPERTY()
    URunArenaData* CurrentArenaData = nullptr;

    // Cached player reference
    UPROPERTY()
    AShooterCharacter* PlayerRef = nullptr;

    // (Reserved for future use: delayed spawn between waves)
    FTimerHandle SpawnTimerHandle;

    // Augment pedestal Blueprint/Class to spawn as reward
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arena|Rewards")
    TSubclassOf<AAugmentPedestal> AugmentPedestalClass;

    // Called when a pedestal broadcasts that the player made a choice
    UFUNCTION()
    void OnAugmentChosen(AAugmentPedestal* PedestalChosen);

    // Spawns the three pedestals in front of the manager
    void SpawnArenaPedestals();

public:

    void SetCurrentArenaData(URunArenaData* InData);
    
    // Entry point called by RunDirector when arena level loads
    UFUNCTION(BlueprintCallable)
    void StartArena();

    // Spawns a specific wave by index
    UFUNCTION(BlueprintCallable)
    void SpawnWave(int32 WaveIndex);

    // Bound to AI OnDestroyed events
    UFUNCTION()
    void OnEnemyDied(AActor* DeadActor);

    // For Blueprint to morph environment between waves
    UFUNCTION(BlueprintImplementableEvent)
    void MorphArenaVisuals();

    // When a wave is completely cleared (no ActiveEnemies left)
    void HandleWaveCleared();

    // When this arena (room) is completely done (all waves + reward taken)
    void HandleArenaEnd();

	// When this arena (room) is the final one and has been cleared
    void HandleArenaVictory();
};

