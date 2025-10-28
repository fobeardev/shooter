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
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void SetupInputComponent() override;

    // ---------------------------------------------------------------------------
    // Input Mapping Context
    // ---------------------------------------------------------------------------
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> IMC_Default;

    // ---------------------------------------------------------------------------
    // Input Actions
    // ---------------------------------------------------------------------------
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Move;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Look;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Jump;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Dash;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Aim;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Fire;

    /** Debug key to self-apply damage for respawn testing */
    UPROPERTY(EditDefaultsOnly, Category = "Input|Debug")
    TObjectPtr<UInputAction> IA_DebugDamage;

private:
    // ---------------------------------------------------------------------------
    // Input Event Handlers
    // ---------------------------------------------------------------------------
    UFUNCTION()
    void OnMove(const FInputActionValue& Value);

    UFUNCTION()
    void OnLook(const FInputActionValue& Value);

    UFUNCTION()
    void OnJumpStarted();

    UFUNCTION()
    void OnJumpCanceled();

    UFUNCTION()
    void OnDashPressed();

    UFUNCTION()
    void OnAim(const FInputActionValue& Value);

    UFUNCTION()
    void OnFire(const FInputActionValue& Value);

    /** Debug: self-damage test */
    UFUNCTION()
    void OnDebugDamagePressed();
};