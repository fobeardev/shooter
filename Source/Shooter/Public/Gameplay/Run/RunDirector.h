#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "RunDirector.generated.h"

class URunDirectorConfig;
class URunArenaData;
class AShooterCharacter;
struct FGameplayEventData;

UCLASS(BlueprintType, Blueprintable)
class SHOOTER_API URunDirector : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    URunDirector();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void StartNewRun();
    void LoadArenaByIndex(int32 Index);
    void ReturnToHub();
    void HandlePlayerDeath();

    void BindRunEventsToCurrentASC();

    void OnArenaLevelLoaded(UWorld* LoadedWorld);
    void OnArenaCleared(const FGameplayEventData* Payload);

    UPROPERTY(EditDefaultsOnly, Category = "Config")
    TSoftObjectPtr<URunDirectorConfig> ConfigAsset;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Run Director")
    TArray<URunArenaData*> ArenaSequence;

private:
    AShooterCharacter* PlayerRef = nullptr;

    int32 CurrentArenaIndex = 0;
    int32 CurrentRunIndex = 0;
    int32 CurrentDifficultyTier = 1;

    // Cached pointer to currently loaded ArenaData
    URunArenaData* CurrentArenaData = nullptr;
};
