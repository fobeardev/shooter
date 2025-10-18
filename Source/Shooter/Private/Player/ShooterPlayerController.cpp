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

    FInputModeGameOnly Mode;
    SetInputMode(Mode);

    if (ULocalPlayer* LP = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsys = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (IMC_Default)
            {
                Subsys->AddMappingContext(IMC_Default, 0);
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
            EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AShooterPlayerController::OnMove);

        if (IA_Look)
            EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AShooterPlayerController::OnLook);

        if (IA_Jump)
        {
            EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &AShooterPlayerController::OnJumpStarted);
            EIC->BindAction(IA_Jump, ETriggerEvent::Canceled, this, &AShooterPlayerController::OnJumpCanceled);
        }

        if (IA_Dash)
            EIC->BindAction(IA_Dash, ETriggerEvent::Started, this, &AShooterPlayerController::OnDashPressed);

        if (IA_Aim)
        {
            EIC->BindAction(IA_Aim, ETriggerEvent::Started, this, &AShooterPlayerController::OnAim);
            EIC->BindAction(IA_Aim, ETriggerEvent::Completed, this, &AShooterPlayerController::OnAim);
        }

        if (IA_Fire)
        {
            EIC->BindAction(IA_Fire, ETriggerEvent::Started, this, &AShooterPlayerController::OnFire);
            EIC->BindAction(IA_Fire, ETriggerEvent::Completed, this, &AShooterPlayerController::OnFire);
        }

        // --- Debug Damage Key (e.g. press "K" to self-damage) ---
        if (IA_DebugDamage)
        {
            EIC->BindAction(IA_DebugDamage, ETriggerEvent::Started, this, &AShooterPlayerController::OnDebugDamagePressed);
        }
    }
}

// ---------------------------------------------------------------------------
// Input Handlers (delegate to ShooterCharacter)
// ---------------------------------------------------------------------------

void AShooterPlayerController::OnMove(const FInputActionValue& Value)
{
    if (AShooterCharacter* C = Cast<AShooterCharacter>(GetPawn()))
        C->Input_Move(Value.Get<FVector2D>());
}

void AShooterPlayerController::OnLook(const FInputActionValue& Value)
{
    if (AShooterCharacter* C = Cast<AShooterCharacter>(GetPawn()))
        C->Input_Look(Value.Get<FVector2D>());
}

void AShooterPlayerController::OnJumpStarted()
{
    if (AShooterCharacter* C = Cast<AShooterCharacter>(GetPawn()))
        C->Jump();
}

void AShooterPlayerController::OnJumpCanceled()
{
    if (AShooterCharacter* C = Cast<AShooterCharacter>(GetPawn()))
        C->StopJumping();
}

void AShooterPlayerController::OnDashPressed()
{
    if (AShooterCharacter* C = Cast<AShooterCharacter>(GetPawn()))
        C->Input_Dash();
}

void AShooterPlayerController::OnAim(const FInputActionValue& Value)
{
    if (AShooterCharacter* C = Cast<AShooterCharacter>(GetPawn()))
        C->Input_Aim(Value);
}

void AShooterPlayerController::OnFire(const FInputActionValue& Value)
{
    if (AShooterCharacter* C = Cast<AShooterCharacter>(GetPawn()))
    {
        const bool bPressed = Value.Get<bool>();
        if (bPressed) C->Input_FirePressed();
        else          C->Input_FireReleased();
    }
}

// ---------------------------------------------------------------------------
// Debug: Self-damage trigger
// ---------------------------------------------------------------------------
void AShooterPlayerController::OnDebugDamagePressed()
{
    if (AShooterCharacter* C = Cast<AShooterCharacter>(GetPawn()))
    {
        C->Debug_ApplySelfDamage();
    }
}
