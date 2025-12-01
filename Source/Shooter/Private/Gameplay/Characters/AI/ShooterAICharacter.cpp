// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/AI/ShooterAICharacter.h"
#include "Gameplay/AI/Controller/ShooterAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

AShooterAICharacter::AShooterAICharacter()
{
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->bOrientRotationToMovement = false;      // do NOT auto-face path
        Move->bUseControllerDesiredRotation = true;   // face where controller points (focus)
        Move->RotationRate = FRotator(0.f, 540.f, 0.f);
    }

    bUseControllerRotationYaw = true;                 // let controller drive yaw
}

void AShooterAICharacter::BeginPlay()
{
    Super::BeginPlay();

    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        // Optional: confirm controller is using ShooterAIController
        UE_LOG(LogTemp, Log, TEXT("%s AI spawned with controller: %s"), *GetName(), *GetNameSafe(AIC));
    }
}
