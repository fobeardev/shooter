// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ShooterMeleeWeapon.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AShooterMeleeWeapon::AShooterMeleeWeapon()
{
    // Base WeaponMeshComponent is already created
    WeaponMeshComponent->SetRelativeLocation(FVector::ZeroVector);
    WeaponMeshComponent->SetRelativeRotation(FRotator::ZeroRotator);
}

void AShooterMeleeWeapon::HandleFire_Internal()
{
    if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
    {
        if (AttackMontage)
        {
            OwnerChar->PlayAnimMontage(AttackMontage);
        }

        // Perform melee trace
        FVector Start = OwnerChar->GetActorLocation() + OwnerChar->GetActorForwardVector() * 50.f;
        FVector End = Start + OwnerChar->GetActorForwardVector() * Range;

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(OwnerChar);

        if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))
        {
            AActor* HitActor = Hit.GetActor();
            if (HitActor)
            {
                UAbilitySystemComponent* SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerChar);
                UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor);

                if (SourceASC && TargetASC)
                {
                    FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
                    Context.AddInstigator(OwnerChar, this);

                    // You can reuse your ballistic GE for now
                    TSubclassOf<UGameplayEffect> DamageGE = LoadClass<UGameplayEffect>(
                        nullptr, TEXT("/Game/Hadeslike/Abilities/GE/GE_Damage_Melee.GE_Damage_Melee_C"));

                    if (DamageGE)
                    {
                        FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageGE, 1.f, Context);
                        if (Spec.IsValid())
                        {
                            Spec.Data->SetSetByCallerMagnitude(
                                FGameplayTag::RequestGameplayTag(TEXT("Data.Weapon.DamageScalar")),
                                -Damage
                            );

                            TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

                            SourceASC->ExecuteGameplayCue(HitCueTag);
                        }
                    }
                }
            }
        }
    }
}
