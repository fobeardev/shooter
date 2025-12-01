// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIProceduralAimerComponent.generated.h"

class USKGProceduralAnimComponent;
class USKGFirearmComponent;
class AShooterCombatCharacter;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SHOOTER_API UAIProceduralAimerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAIProceduralAimerComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Target actor to aim at (set from AI controller or blackboard) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
    AActor* TargetActor;

    /** Whether AI should currently aim procedurally */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
    bool bEnableAiming = true;

    /** Max rotation speed when tracking target */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
    float AimInterpSpeed = 8.0f;

    /** Debug draw aiming line */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawDebug = false;

protected:
    UPROPERTY()
    AShooterCombatCharacter* OwnerCharacter;

    UPROPERTY()
    USKGProceduralAnimComponent* ProceduralComp;

    UPROPERTY()
    USKGFirearmComponent* FirearmComp;

    /** Cached current aim rotation */
    FRotator CurrentAimRotation;
};
