// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReprintPodActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class SHOOTER_API AReprintPodActor : public AActor
{
    GENERATED_BODY()

public:
    AReprintPodActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* PodMesh;

    // Optional effect origin for spawn or cinematic
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USceneComponent* SpawnPoint;
};
