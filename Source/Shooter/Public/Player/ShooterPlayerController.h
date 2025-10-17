#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "ShooterPlayerController.generated.h"

// ---------------------------------------------------------------------------
// Forward Declarations
// ---------------------------------------------------------------------------
class UInputMappingContext;
class UInputAction;

/**
 * Handles Enhanced Input routing for AShooterCharacter.
 *
 * Responsibilities:
 *  - Registers the default input mapping context.
 *  - Binds input actions (Move, Look, Jump, Dash, Aim, Fire).
 *  - Forwards input events to AShooterCharacter.
 *
 * This keeps the character class free of direct Enhanced Input logic,
 * ensuring clear separation between player input and gameplay behavior.
 */
UCLASS()
class SHOOTER_API AShooterPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AShooterPlayerController();

protected:
    // ---------------------------------------------------------------------------
    // Core Overrides
    // ---------------------------------------------------------------------------
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    // ---------------------------------------------------------------------------
    // Input Mapping Context
    // ---------------------------------------------------------------------------
    /** Mapping context applied on BeginPlay to this local player. */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> IMC_Default;

    // ---------------------------------------------------------------------------
    // Input Actions
    // ---------------------------------------------------------------------------
    /** WASD or Left Stick */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Move;

    /** Mouse or Right Stick Look */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Look;

    /** Spacebar or Jump Button */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Jump;

    /** Left Alt or Controller Button */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Dash;

    /** Right Mouse Button (Aim Down Sights) */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Aim;

    /** Left Mouse Button (Fire Weapon) */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Fire;

private:
    // ---------------------------------------------------------------------------
    // Input Event Handlers
    // ---------------------------------------------------------------------------

    /** Movement vector (X = forward/back, Y = strafe). */
    UFUNCTION()
    void OnMove(const FInputActionValue& Value);

    /** Look delta from mouse or stick. */
    UFUNCTION()
    void OnLook(const FInputActionValue& Value);

    /** Jump pressed. */
    UFUNCTION()
    void OnJumpStarted();

    /** Jump released/canceled. */
    UFUNCTION()
    void OnJumpCanceled();

    /** Dash triggered. */
    UFUNCTION()
    void OnDashPressed();

    /** Aim pressed/released (Boolean). */
    UFUNCTION()
    void OnAim(const FInputActionValue& Value);

    /** Fire pressed/released (Boolean). */
    UFUNCTION()
    void OnFire(const FInputActionValue& Value);
};
