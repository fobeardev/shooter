// ShooterCharacterMovement_DOOM.h
#pragma once

#include "CoreMinimal.h"
#include "Components/SKGCharacterMovementComponent.h"
#include "ShooterCharacterMovement_DOOM.generated.h"

/**
 * Fast, DOOM-style character movement built on SKG network layer.
 * Emphasizes responsiveness, air control, and snappy jump arcs.
 */
UCLASS()
class SHOOTER_API UShooterCharacterMovement_Doom : public USKGCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UShooterCharacterMovement_Doom();

protected:
	virtual void BeginPlay() override;
	virtual void PhysFalling(float DeltaTime, int32 Iterations) override;
	virtual float GetMaxSpeed() const override;
};
