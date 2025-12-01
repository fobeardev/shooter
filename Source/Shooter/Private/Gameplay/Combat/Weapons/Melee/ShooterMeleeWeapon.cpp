// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Combat/Weapons/Melee/ShooterMeleeWeapon.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

#include "Components/SKGProceduralAnimComponent.h"

static TAutoConsoleVariable<int32> CVarMeleeDebug(
    TEXT("colosseum.MeleeDebug"),
    0,
    TEXT("Enable melee weapon debug visuals (0 = off, 1 = on)"),
    ECVF_Cheat);

AShooterMeleeWeapon::AShooterMeleeWeapon()
{
    WeaponMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMeshComponent;

    if (WeaponMeshComponent)
    {
        WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        WeaponMeshComponent->SetGenerateOverlapEvents(false);
        WeaponMeshComponent->SetIsReplicated(true);
        WeaponMeshComponent->SetRelativeLocation(FVector::ZeroVector);
        WeaponMeshComponent->SetRelativeRotation(FRotator::ZeroRotator);
    }
}

void AShooterMeleeWeapon::HandleFire_Internal()
{
    if (bIsSwinging)
        return; // prevent spam / double-fire until animation finishes

    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar)
        return;

    bIsSwinging = true;

    // Play swing montage
    if (AttackMontage)
    {
        float Duration = OwnerChar->PlayAnimMontage(AttackMontage);
        if (Duration > 0.f)
        {
            FTimerHandle ResetTimer;
            GetWorld()->GetTimerManager().SetTimer(ResetTimer, [this]()
                {
                    bIsSwinging = false;
                    if (ProceduralAnimComponent)
                    {
                        // Return to procedural holding pose
                        ProceduralAnimComponent->InitializeProceduralAnimComponent();
                    }
                }, Duration, false);
        }
    }

    // Perform melee trace during swing
    FVector Start = OwnerChar->GetActorLocation() + OwnerChar->GetActorForwardVector() * 50.f;
    FVector End = Start + OwnerChar->GetActorForwardVector() * Range;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(OwnerChar);

    // === Debug: visualize trace line ===
    const bool bDrawDebug = true; // toggle this when needed

    if (bDrawDebug)
    {
        DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 2.0f, 0, 1.5f);
        DrawDebugSphere(GetWorld(), Start, 8.f, 12, FColor::Green, false, 2.0f);
        DrawDebugSphere(GetWorld(), End, 8.f, 12, FColor::Red, false, 2.0f);
    }

    // === Line trace ===
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))
    {
        AActor* HitActor = Hit.GetActor();

        if (bDrawDebug)
        {
            // Mark impact point
            DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 12.f, 16, FColor::Cyan, false, 2.0f);
            UE_LOG(LogTemp, Warning, TEXT("[Melee] Hit: %s | Distance: %.1f"),
                *GetNameSafe(HitActor),
                FVector::Dist(Start, Hit.ImpactPoint));
        }

        if (HitActor && DamageEffect)
        {
            UAbilitySystemComponent* SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerChar);
            UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor);

            if (SourceASC && TargetASC)
            {
                FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
                Context.AddInstigator(OwnerChar, this);

                FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffect, 1.f, Context);
                if (Spec.IsValid())
                {
                    Spec.Data->SetSetByCallerMagnitude(
                        FGameplayTag::RequestGameplayTag(TEXT("Data.Weapon.DamageScalar")),
                        -Damage
                    );

                    TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
                    SourceASC->ExecuteGameplayCue(HitCueTag);

                    if (bDrawDebug)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("[Melee] Applied %.1f damage to %s"), Damage, *GetNameSafe(HitActor));
                    }
                }
                else if (bDrawDebug)
                {
                    UE_LOG(LogTemp, Warning, TEXT("[Melee] Spec invalid — could not apply damage to %s"), *GetNameSafe(HitActor));
                }
            }
        }
    }
    else if (bDrawDebug)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Melee] Missed. No actor hit within %.1f units"), Range);
    }

}

void AShooterMeleeWeapon::HandleStopFire_Internal()
{
    // Reset swing lock manually if interrupted
    bIsSwinging = false;
    if (ProceduralAnimComponent)
    {
        ProceduralAnimComponent->InitializeProceduralAnimComponent();
    }
}
