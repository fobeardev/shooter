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

    /** Digital action bound to Left Alt in the IMC */
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_Dash;

private:
    void OnDashPressed();   // calls Character->Input_Dash()
};
