#include "Player/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Characters/ShooterCharacter.h"

AShooterPlayerController::AShooterPlayerController() {}

void AShooterPlayerController::BeginPlay()
{
    Super::BeginPlay();

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
        if (IA_Dash)
        {
            EIC->BindAction(IA_Dash, ETriggerEvent::Started, this, &AShooterPlayerController::OnDashPressed);
        }
    }
}

void AShooterPlayerController::OnDashPressed()
{
    if (AShooterCharacter* C = Cast<AShooterCharacter>(GetPawn()))
    {
        C->Input_Dash();
    }
}
