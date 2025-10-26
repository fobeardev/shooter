// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AIProceduralAimerComponent.h"
#include "Characters/ShooterCombatCharacter.h"
#include "Weapons/ShooterWeaponBase.h"
#include "Components/SKGProceduralAnimComponent.h"
#include "Components/SKGFirearmComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UAIProceduralAimerComponent::UAIProceduralAimerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UAIProceduralAimerComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<AShooterCombatCharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("[AIProceduralAimerComponent] Owner is not AShooterCombatCharacter."));
        return;
    }

    // Find weapon procedural components
    AActor* Weapon = Cast<AActor>(OwnerCharacter->GetEquippedWeapon());

    if (!Weapon)
    {
        TArray<AActor*> AttachedActors;
        OwnerCharacter->GetAttachedActors(AttachedActors);
        if (AttachedActors.Num() > 0)
        {
            Weapon = AttachedActors[0];
        }
    }

    if (Weapon)
    {
        ProceduralComp = Weapon->FindComponentByClass<USKGProceduralAnimComponent>();
        FirearmComp = Weapon->FindComponentByClass<USKGFirearmComponent>();

        if (FirearmComp)
        {
            FirearmComp->Held(); // simulate being held by pawn
        }
    }
}

void UAIProceduralAimerComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bEnableAiming || !TargetActor || !ProceduralComp || !OwnerCharacter)
        return;

    // Compute desired look rotation toward target
    const FVector EyeLocation = OwnerCharacter->GetMesh()->GetSocketLocation(FName("head"));
    const FVector TargetLocation = TargetActor->GetActorLocation();
    const FVector AimDir = (TargetLocation - EyeLocation).GetSafeNormal();
    const FRotator TargetRot = AimDir.Rotation();

    // Smooth the aim rotation (just for feel, optional)
    CurrentAimRotation = FMath::RInterpTo(CurrentAimRotation, TargetRot, DeltaTime, AimInterpSpeed);

    // Apply orientation directly to the AI's control rotation (optional)
    OwnerCharacter->SetActorRotation(FRotator(0.f, CurrentAimRotation.Yaw, 0.f));

    // Update procedural aiming relative to AI’s mesh
    ProceduralComp->UpdateAimOffset(OwnerCharacter->GetMesh());

    if (bDrawDebug)
    {
        DrawDebugLine(GetWorld(), EyeLocation, EyeLocation + AimDir * 1000.0f, FColor::Green, false, -1.f, 0, 1.f);
    }

    // Engage aiming mode if available
    if (FirearmComp && !FirearmComp->IsPointAiming())
    {
        FirearmComp->StartPointAiming(true);
    }
}
