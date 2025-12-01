#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterProjectile.generated.h"

UCLASS()
class SHOOTER_API AShooterProjectile : public AActor
{
    GENERATED_BODY()

public:
    AShooterProjectile();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* Mesh;

    virtual void BeginPlay() override;
};
