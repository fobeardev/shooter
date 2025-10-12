#pragma once

#include "CoreMinimal.h"
#include "ShooterFirearm.generated.h"

// Forward declarations of SKG components
class USKGFirearmComponent;
class USKGAttachmentManagerComponent;
class USKGProceduralAnimComponent;
class USKGOffhandIKComponent;
class USKGMuzzleComponent;
class USkeletalMeshComponent;

UCLASS()
class SHOOTER_API AShooterFirearm : public AActor
{
    GENERATED_BODY()

public:
    AShooterFirearm();

protected:
    virtual void BeginPlay() override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // ------------------------------
    // Components
    // ------------------------------
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Firearm")
    TObjectPtr<USkeletalMeshComponent> FirearmMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Firearm")
    TObjectPtr<USKGFirearmComponent> FirearmComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Firearm")
    TObjectPtr<USKGAttachmentManagerComponent> AttachmentManagerComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Firearm")
    TObjectPtr<USKGProceduralAnimComponent> ProceduralAnimComponent;

    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Firearm")
    //TObjectPtr<USKGMuzzleComponent> MuzzleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Firearm")
    TObjectPtr<USKGOffhandIKComponent> OffhandIKComponent;

private:

    void LogComponentInitialization() const;
};
