// Fill out your copyright notice in the Description page of Project Settings.


#include "World/Hub/ReprintPodActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

AReprintPodActor::AReprintPodActor()
{
    Root = CreateDefaultSubobject<USceneComponent>("Root");
    SetRootComponent(Root);

    PodMesh = CreateDefaultSubobject<UStaticMeshComponent>("PodMesh");
    PodMesh->SetupAttachment(Root);

    SpawnPoint = CreateDefaultSubobject<USceneComponent>("SpawnPoint");
    SpawnPoint->SetupAttachment(Root);
}
