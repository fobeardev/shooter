// Fill out your copyright notice in the Description page of Project Settings.


#include "World/Hub/CloneFacilityManager.h"
#include "World/Hub/ReprintPodActor.h"
#include "Gameplay/Run/RunDirector.h"
#include "Gameplay/Characters/Player/ShooterCharacter.h"
#include "Gameplay/Tags/ShooterGameplayTags.h"

#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"

ACloneFacilityManager::ACloneFacilityManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ACloneFacilityManager::BeginPlay()
{
    Super::BeginPlay();

    // Prefer GetPlayerCharacter to ensure you get a Character (movement etc.)
    PlayerRef = Cast<AShooterCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!PlayerRef)
    {
        PlayerRef = Cast<AShooterCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
    }

    // If the pod was not set in the editor, try to find one
    if (!ReprintPod)
    {
        for (TActorIterator<AReprintPodActor> It(GetWorld()); It; ++It)
        {
            ReprintPod = *It;
            break;
        }
    }

    // Small delay: ensures pawn, movement, nav, etc. are fully initialized
    if (ReprintPod && PlayerRef)
    {
        FTimerHandle TmpHandle;
        GetWorldTimerManager().SetTimer(
            TmpHandle,
            this,
            &ACloneFacilityManager::BeginReprintSequence,
            0.05f,
            false
        );
    }
}

void ACloneFacilityManager::BeginReprintSequence()
{
    UE_LOG(LogTemp, Log, TEXT("CloneFacility: Starting reprint sequence."));

    // Blueprint event for cinematic or cue
    OnReprintCinematicStart();

    // Optional: trigger GameplayCue.Clone.Reprint for VFX/SFX
    if (PlayerRef)
    {
        FGameplayEventData Data;
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
            PlayerRef,
            ShooterTags::GameplayCue_Clone_Reprint,
            Data
        );
    }

    // Simple respawn logic
    if (ReprintPod && PlayerRef)
    {
        const FVector SpawnLoc = ReprintPod->SpawnPoint
            ? ReprintPod->SpawnPoint->GetComponentLocation()
            : ReprintPod->GetActorLocation();

        const FRotator SpawnRot = ReprintPod->GetActorRotation();

        // Optional: stop movement before teleport to avoid residual velocities
        if (UCharacterMovementComponent* Move = PlayerRef->GetCharacterMovement())
        {
            Move->StopMovementImmediately();
        }

        PlayerRef->SetActorLocationAndRotation(SpawnLoc, SpawnRot);

        // Re-enable walking to avoid edge cases if physics or falling got engaged
        if (UCharacterMovementComponent* Move = PlayerRef->GetCharacterMovement())
        {
            Move->SetMovementMode(MOVE_Walking);
        }
    }

    // Blueprint event: cinematic end hook
    OnReprintCinematicEnd();

    UE_LOG(LogTemp, Log, TEXT("CloneFacility: Reprint complete."));
}

void ACloneFacilityManager::BeginRun()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("CloneFacilityManager: No world available."));
        return;
    }

    URunDirector* RunDirector = GetWorld()->GetGameInstance()->GetSubsystem<URunDirector>();

    if (!RunDirector)
    {
        UE_LOG(LogTemp, Error, TEXT("CloneFacilityManager: RunDirector subsystem not found!"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("CloneFacilityManager: Starting run via RunDirector."));

    RunDirector->StartNewRun();
}

void ACloneFacilityManager::SaveGenomeData()
{
    UE_LOG(LogTemp, Log, TEXT("CloneFacility: Saving genome data stub."));
    // TODO: Save genome or run meta progression here
}

void ACloneFacilityManager::LoadGenomeData()
{
    UE_LOG(LogTemp, Log, TEXT("CloneFacility: Loading genome data stub."));
    // TODO: Load genome or player upgrades here
}
