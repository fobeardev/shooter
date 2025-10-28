// Fill out your copyright notice in the Description page of Project Settings.


// ShooterCharacterMovement_Doom.cpp
#include "Player/Components/ShooterCharacterMovement_Doom.h"
#include "GameFramework/Character.h"

UShooterCharacterMovement_Doom::UShooterCharacterMovement_Doom()
{
	// Base walking and sprint speeds
	MaxWalkSpeed = 500.f;
	MaxSprintSpeed = 850.f;

	MaxAcceleration = 4096.f;              // rocket-fast acceleration
	BrakingDecelerationWalking = 2048.f;
	BrakingDecelerationFalling = 0.f;

	GroundFriction = 4.f;                  // allow power-slides
	BrakingFriction = 1.f;
	AirControl = 0.95f;                    // tight steering midair
	AirControlBoostMultiplier = 0.f;

	JumpZVelocity = 780.f;                 // snappy, high jump
	GravityScale = 2.0f;                   // hard fall for punchy landings
	bUseSeparateBrakingFriction = false;

	bOrientRotationToMovement = false;
	bUseControllerDesiredRotation = true;

	MovementMode = MOVE_Walking;

	RotationRate = FRotator(0.f, 540.f, 0.f);
	MaxWalkSpeed = 600.f;
}

float UShooterCharacterMovement_Doom::GetMaxSpeed() const
{
	// Use SKG sprint flag system
	if (MovementMode == MOVE_Walking || MovementMode == MOVE_NavWalking)
	{
		return bWantsToSprint ? MaxSprintSpeed : MaxWalkSpeed;
	}

	if (MovementMode == MOVE_Falling)
	{
		// keep high momentum in air
		return FMath::Max(MaxSprintSpeed, MaxWalkSpeed * 1.2f);
	}
	return Super::GetMaxSpeed();
}

void UShooterCharacterMovement_Doom::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("Movement initialized. UpdatedComponent=%s"), *GetNameSafe(UpdatedComponent));
}

void UShooterCharacterMovement_Doom::PhysFalling(float DeltaTime, int32 Iterations)
{
	// Preserve Doom-style lateral velocity
	const FVector PrevVel = Velocity;
	Super::PhysFalling(DeltaTime, Iterations);

	const FVector HorizPrev = FVector(PrevVel.X, PrevVel.Y, 0.f);
	const FVector HorizNow = FVector(Velocity.X, Velocity.Y, 0.f);

	// Restore lost horizontal momentum at jump apex
	if (HorizNow.SizeSquared() < HorizPrev.SizeSquared() * 0.8f)
	{
		const FVector Restored = FMath::Lerp(HorizNow, HorizPrev, 0.25f);
		Velocity.X = Restored.X;
		Velocity.Y = Restored.Y;
	}
}
