#include "Player/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Characters/ShooterCharacter.h"

AShooterPlayerController::AShooterPlayerController()
{
    bShowMouseCursor = false;
}

void AShooterPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Game-only input mode for typical shooter controls
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

// --------------------------------
// SetupInputComponent: bind Enhanced Input actions
// --------------------------------
void AShooterPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
    {
        // --- Movement ---
        if (IA_Move)
        {
            EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AShooterPlayerController::OnMove);
        }

        // --- Look ---
        if (IA_Look)
        {
            EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AShooterPlayerController::OnLook);
        }

        // --- Jump ---
        if (IA_Jump)
        {
            EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &AShooterPlayerController::OnJumpStarted);
            EIC->BindAction(IA_Jump, ETriggerEvent::Canceled, this, &AShooterPlayerController::OnJumpCanceled);
        }

        // --- Dash ---
        if (IA_Dash)
        {
            EIC->BindAction(IA_Dash, ETriggerEvent::Started, this, &AShooterPlayerController::OnDashPressed);
        }

        // --- Aim (RMB / ADS) ---
        if (IA_Aim)
        {
            EIC->BindAction(IA_Aim, ETriggerEvent::Started, this, &AShooterPlayerController::OnAim);
            EIC->BindAction(IA_Aim, ETriggerEvent::Completed, this, &AShooterPlayerController::OnAim);
        }

        // --- Fire (LMB) ---
        if (IA_Fire)
        {
            EIC->BindAction(IA_Fire, ETriggerEvent::Started, this, &AShooterPlayerController::OnFire);
            EIC->BindAction(IA_Fire, ETriggerEvent::Completed, this, &AShooterPlayerController::OnFire);
        }
    }
}

// --------------------------------
// Input Handlers (delegate to ShooterCharacter)
// --------------------------------

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

void AShooterPlayerController::OnAim(const FInputActionValue& Value)
{
    if (AShooterCharacter* C = Cast<AShooterCharacter>(GetPawn()))
    {
        // Forward to character's SKG-integrated aim function
        C->Input_Aim(Value);
    }
}

void AShooterPlayerController::OnFire(const FInputActionValue& Value)
{
    if (AShooterCharacter* C = Cast<AShooterCharacter>(GetPawn()))
    {
        const bool bPressed = Value.Get<bool>();

        if (bPressed)
        {
            C->Input_FirePressed();
        }
        else
        {
            C->Input_FireReleased();
        }
    }
}
