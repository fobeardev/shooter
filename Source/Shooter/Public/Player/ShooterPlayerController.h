#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class SHOOTER_API AShooterPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    AShooterPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    /** Added to owning local player on begin play */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> IMC_Default;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Move;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Jump;

    /** Digital action bound to Mouse X / Y in the IMC */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Look;

    /** Digital action bound to Left Alt in the IMC */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Dash;


private:
    UFUNCTION()
    void OnDashPressed();

    UFUNCTION()
    void OnLook(const FInputActionValue& Value);

    UFUNCTION()
    void OnMove(const FInputActionValue& Value);

    UFUNCTION() 
    void OnJumpStarted();

    UFUNCTION()
    void OnJumpCanceled();
};
