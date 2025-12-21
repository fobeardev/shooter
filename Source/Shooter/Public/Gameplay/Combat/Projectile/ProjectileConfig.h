#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

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

    // Bullet-hell emission
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Pattern")
    int32 ProjectileCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Pattern")
    float SpreadHalfAngleDeg = 0.0f;

    // Ricochet / Pierce
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Modifier")
    int32 MaxRicochets = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Modifier")
    int32 MaxPierces = 0;

    // Acceleration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Modifier")
    float AccelerationCmPerSec2 = 0.0f;

    // Homing (simple but real)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Modifier")
    float HomingAcquireRadiusCm = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Modifier")
    float HomingTurnRateDegPerSec = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    float Damage = 20.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
    float BulletMassKg = 0.0f;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
    TSoftClassPtr<class UGameplayEffect> DamageGameplayEffectClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
    FGameplayTag SetByCallerDamageTag;
};
