#include "Player/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Characters/ShooterCharacter.h"

AShooterPlayerController::AShooterPlayerController() {}

void AShooterPlayerController::BeginPlay()
{
    Super::BeginPlay();

    SetShowMouseCursor(false);

    FInputModeGameOnly Mode; 
    SetInputMode(Mode);

    if (ULocalPlayer* LP = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (IMC_Default)
            {
                Subsys->AddMappingContext(IMC_Default, /*Priority*/0);
            }
        }
    }
}

void AShooterPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (IA_Move)
        {
            EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AShooterPlayerController::OnMove);
        }

        if (IA_Look)
        {
            EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AShooterPlayerController::OnLook);
        }

        if (IA_Jump)
        {
            EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &AShooterPlayerController::OnJumpStarted);
            EIC->BindAction(IA_Jump, ETriggerEvent::Canceled, this, &AShooterPlayerController::OnJumpCanceled);
        }

        if (IA_Dash)
        {
            EIC->BindAction(IA_Dash, ETriggerEvent::Started, this, &AShooterPlayerController::OnDashPressed);
        }
    }
}

void AShooterPlayerController::OnMove(const FInputActionValue& Value)
{
    if (AShooterCharacter* C = Cast<AShooterCharacter>(GetPawn()))
    {
        const FVector2D Axis = Value.Get<FVector2D>();
        C->Input_Move(Axis);
    }
}

void AShooterPlayerController::OnLook(const FInputActionValue& Value)
{
    if (AShooterCharacter* C = Cast<AShooterCharacter>(GetPawn()))
    {
        const FVector2D Axis = Value.Get<FVector2D>();
        C->Input_Look(Axis);
    }
}

void AShooterPlayerController::OnJumpStarted()
{
    if (AShooterCharacter* C = Cast<AShooterCharacter>(GetPawn()))
    {
        C->Jump();
    }
}

void AShooterPlayerController::OnJumpCanceled()
{
    if (AShooterCharacter* C = Cast<AShooterCharacter>(GetPawn()))
    {
        C->StopJumping();
    }
}

void AShooterPlayerController::OnDashPressed()
{
    if (AShooterCharacter* C = Cast<AShooterCharacter>(GetPawn()))
    {
        C->Input_Dash();
    }
}
