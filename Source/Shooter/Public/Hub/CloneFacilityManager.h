// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CloneFacilityManager.generated.h"

class AShooterCharacter;
class AReprintPodActor;
class URunDirector;

UCLASS(BlueprintType)
class SHOOTER_API ACloneFacilityManager : public AActor
{
    GENERATED_BODY()

public:
    ACloneFacilityManager();

protected:
    virtual void BeginPlay() override;

    // Reference to the reprint pod where the player respawns
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clone Facility")
    AReprintPodActor* ReprintPod;

    // Optional terminal or portal actor to begin a new run
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clone Facility")
    AActor* RunTerminal;

    // Reference to run director for run transitions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clone Facility")
    URunDirector* RunDirectorRef;

    // Cached player pawn
    UPROPERTY()
    AShooterCharacter* PlayerRef;

public:
    // === Reprint ===
    UFUNCTION(BlueprintCallable, Category = "Clone Facility")
    void BeginReprintSequence();

    UFUNCTION(BlueprintImplementableEvent, Category = "Clone Facility")
    void OnReprintCinematicStart();

    UFUNCTION(BlueprintImplementableEvent, Category = "Clone Facility")
    void OnReprintCinematicEnd();

    // === Interaction ===
    UFUNCTION(BlueprintCallable, Category = "Clone Facility")
    void BeginRun();

    // === Save/Load Stubs ===
    UFUNCTION(BlueprintCallable, Category = "Clone Facility")
    void SaveGenomeData();

    UFUNCTION(BlueprintCallable, Category = "Clone Facility")
    void LoadGenomeData();
};
