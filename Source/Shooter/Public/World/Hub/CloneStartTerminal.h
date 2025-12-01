#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CloneStartTerminal.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UWidgetComponent;
class ACloneFacilityManager;

UCLASS()
class SHOOTER_API ACloneStartTerminal : public AActor
{
    GENERATED_BODY()

public:
    ACloneStartTerminal();

protected:
    virtual void BeginPlay() override;

    // Components
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* TerminalMesh;

    UPROPERTY(VisibleAnywhere)
    UBoxComponent* InteractionZone;

    UPROPERTY(VisibleAnywhere)
    UWidgetComponent* InteractionPrompt;

    // Reference to facility manager
    UPROPERTY()
    ACloneFacilityManager* FacilityManagerRef = nullptr;

    // prevent multiple triggers while player stays overlapping
    bool bHasTriggeredRun = false;

    // Overlap handler
    UFUNCTION()
    void OnInteract(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

public:
    UFUNCTION(BlueprintCallable)
    void BeginRun();
};
