#pragma once

#include "CoreMinimal.h"
#include "ProjectileConfig.generated.h"

USTRUCT(BlueprintType)
struct FProjectileConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    float InitialSpeed = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    float MaxLifeTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    float Radius = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    float GravityScale = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    float Damage = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    TSubclassOf<UDamageType> DamageTypeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_GameTraceChannel1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    bool bUseSphereTrace = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    bool bSpawnTracerFX = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
    TSoftObjectPtr<class UNiagaraSystem> TracerFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
    TSoftObjectPtr<class UNiagaraSystem> ImpactFX;
};
