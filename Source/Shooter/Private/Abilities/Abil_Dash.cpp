#include "Abilities/Abil_Dash.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameplayEffect.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"
#include "Characters/ShooterCharacter.h"

UAbil_Dash::UAbil_Dash()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    Tag_Ability_Dash = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Dash"));

    SetAssetTags(FGameplayTagContainer(Tag_Ability_Dash));

    ActivationOwnedTags.AddTag(Tag_Ability_Dash);

    Cue_DashStart = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Dash.Start"));

    // ALLOW retrigger while active; our CanActivate gating prevents spam/stacking.
    bRetriggerInstancedAbility = true;

    UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: Ctor: InstancedPerActor, LocalPredicted (Retrigger=TRUE)"));
}


UWorld* UAbil_Dash::GetSafeWorld(const FGameplayAbilityActorInfo* ActorInfo) const
{
    if (ActorInfo && ActorInfo->AvatarActor.IsValid()) { return ActorInfo->AvatarActor->GetWorld(); }
    return nullptr;
}

bool UAbil_Dash::CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags,
    OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    const bool bSuper = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
    const UWorld* World = GetSafeWorld(ActorInfo);
    const double Now = World ? World->GetTimeSeconds() : 0.0;
    const double SinceLast = Now - LastDashStartTime;

    // Debounce
    bool bDebounceOK = (SinceLast >= (double)MinRetriggerGap);

    // Chain window gating
    bool bChainOK = true;
    if (bDashActive)
    {
        if (DashesThisWindow >= 1) bChainOK = false; // already chained once this window
        else if (SinceLast < (double)ChainUnlockDelay) bChainOK = false;
    }

    bool bChargesAllow = true;
    if (bUseCharges)
    {
        const AShooterCharacter* S = ActorInfo ? Cast<AShooterCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
        bChargesAllow = (S && S->HasDashCharge());
        UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: CanActivate: charges allow=%d  curr=%d/%d"),
            bChargesAllow ? 1 : 0, S ? S->GetCurrentDashCharges() : -1, S ? S->GetMaxDashCharges() : -1);
    }

    UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: CanActivate: super=%d active=%d dashesWin=%d sinceLast=%.3f gap=%.3f unlock=%.3f"),
        bSuper ? 1 : 0, bDashActive ? 1 : 0, DashesThisWindow, SinceLast, MinRetriggerGap, ChainUnlockDelay);

    return bSuper && bDebounceOK && bChainOK && bChargesAllow;
}

void UAbil_Dash::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilitySpec& Spec)
{
    Super::OnGiveAbility(ActorInfo, Spec);
    UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: OnGiveAbility"));
}

void UAbil_Dash::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilitySpec& Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);
    UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: OnAvatarSet"));
}

void UAbil_Dash::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: Activate: enter"));

    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid()) { EndAbility(Handle, ActorInfo, ActivationInfo, true, true); return; }

    ACharacter* Char = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
    if (!Char) { EndAbility(Handle, ActorInfo, ActivationInfo, true, true); return; }

    UCharacterMovementComponent* Move = Char->GetCharacterMovement();
    if (!Move) { EndAbility(Handle, ActorInfo, ActivationInfo, true, true); return; }

    AShooterCharacter* ShooterChar = Cast<AShooterCharacter>(Char);
    if (!ShooterChar) { EndAbility(Handle, ActorInfo, ActivationInfo, true, true); return; }

    if (bUseCharges)
    {
        if (!ShooterChar->ConsumeDashCharge())
        {
            EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
            return;
        }
    }

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        if (bUseCharges && !ShooterChar->HasAuthority())
        {
            ShooterChar->AddDashChargeLocal(1);
        }
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // --- chain window state ---
    if (const UWorld* World = GetSafeWorld(ActorInfo))
    {
        const double Now = World->GetTimeSeconds();
        if (!bDashActive) { bDashActive = true; DashesThisWindow = 0; UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: Activate: new window start (Now=%.3f)"), Now); }
        LastDashStartTime = Now;
    }

    DashesThisWindow = FMath::Clamp(DashesThisWindow + 1, 0, 2);
    UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: Activate: DashesThisWindow=%d (active=%d)"), DashesThisWindow, bDashActive ? 1 : 0);

    // Direction
    FVector Dir = FVector::ZeroVector;
    if (!Move->GetCurrentAcceleration().IsNearlyZero(1.f))
    {
        Dir = Move->GetCurrentAcceleration().GetSafeNormal2D();
        UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: Dir from Accel -> (%.2f, %.2f)"), Dir.X, Dir.Y);
    }
    else
    {
        FRotator ControlRot = FRotator::ZeroRotator;
        if (AController* C = Char->GetController()) { ControlRot = C->GetControlRotation(); }
        const FRotator YawOnly(0.f, ControlRot.Yaw, 0.f);
        Dir = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X);
        UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: Dir from ControlYaw -> (%.2f, %.2f)"), Dir.X, Dir.Y);
    }
    Dir.Z = 0.f; Dir = Dir.GetSafeNormal();

    // Launch build
    const bool bIsFalling = Move->IsFalling();
    float Speed = DashSpeed;

    if (bIsFalling && bAllowAirDash)
    {
        Speed *= AirDashSpeedMultiplier;

        SavedFallingLateralFriction = Move->FallingLateralFriction;
        SavedBrakingDecelFalling = Move->BrakingDecelerationFalling;
        SavedAirControl = Move->AirControl;

        Move->FallingLateralFriction = AirDashFallingLateralFriction;
        Move->BrakingDecelerationFalling = AirDashBrakingDecel;
        Move->AirControl = AirControlDuringAirDash;
    }

    UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: %sDash: Speed=%.1f"),
        bIsFalling ? TEXT("Air") : TEXT("Ground"), Speed);

    const FVector DashXY = Dir * Speed;

    float PreserveZ = Move->Velocity.Z;
    if (bIsFalling) { PreserveZ = FMath::Min(PreserveZ, MaxUpwardVelDuringAirDash); }

    UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: Pre-Launch Vel: (%.1f, %.1f, %.1f) Falling=%d PresZ=%.1f"),
        Move->Velocity.X, Move->Velocity.Y, Move->Velocity.Z, bIsFalling ? 1 : 0, PreserveZ);

    SavedBrakingFriction = Move->BrakingFrictionFactor;
    if (!bIsFalling) { Move->BrakingFrictionFactor = 0.f; }

    if (bIsFalling)
    {
        Char->LaunchCharacter(FVector(DashXY.X, DashXY.Y, 0.f), false, false);
    }
    else
    {
        Char->LaunchCharacter(FVector(DashXY.X, DashXY.Y, PreserveZ), true, false);
    }

    UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: Post-Launch Vel: (%.1f, %.1f, %.1f)"),
        Move->Velocity.X, Move->Velocity.Y, Move->Velocity.Z);

    // I-Frames
    if (GE_IFrames && ActorInfo->AbilitySystemComponent.IsValid())
    {
        FGameplayEffectSpecHandle Spec =
            ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(
                GE_IFrames, 1.f, ActorInfo->AbilitySystemComponent->MakeEffectContext());

        if (Spec.IsValid())
        {
            Spec.Data->SetDuration(IFrameDuration, true);
            ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
            UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: Applied GE_IFrames for %.2fs"), IFrameDuration);
        }
    }

    // Cue (optional)
    if (ActorInfo->AbilitySystemComponent.IsValid() && Cue_DashStart.IsValid())
    {
        ActorInfo->AbilitySystemComponent->ExecuteGameplayCue(Cue_DashStart);
        UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: Cue executed: %s"), *Cue_DashStart.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: Cue: tag/ASC invalid"));
    }

    // End
    if (UWorld* World = GetSafeWorld(ActorInfo))
    {
        // If this is a chained dash, extend the "end" to the latest activation
        World->GetTimerManager().ClearTimer(EndTimer);
        World->GetTimerManager().SetTimer(EndTimer, this, &UAbil_Dash::EndDash, DashDuration, false);
        UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: EndTimer set: %.2fs (reset)"), DashDuration);
    }

    if (bUseCharges) { ShooterChar->EnsureDashRechargeRunning(); }
}

void UAbil_Dash::EndDash()
{
    UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: EndDash -> EndAbility()"));
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}

void UAbil_Dash::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility, bool bWasCancelled)
{
    UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: EndAbility: begin (active=%d dashesWin=%d)"),
        bDashActive ? 1 : 0, DashesThisWindow);

    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        if (ACharacter* Char = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
        {
            if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
            {
                Move->FallingLateralFriction = SavedFallingLateralFriction >= 0.f ? SavedFallingLateralFriction : 0.f;
                Move->BrakingDecelerationFalling = SavedBrakingDecelFalling >= 0.f ? SavedBrakingDecelFalling : 0.f;
                Move->AirControl = SavedAirControl >= 0.f ? SavedAirControl : 0.6f;  // restore sensible default
                Move->BrakingFrictionFactor = SavedBrakingFriction >= 0.f ? SavedBrakingFriction : 1.f;

                Move->bOrientRotationToMovement = false;
                Char->bUseControllerRotationYaw = true;

                if (Move->MovementMode == MOVE_Falling)
                {
                    Move->FallingLateralFriction = 0.f;
                    Move->BrakingDecelerationFalling = 0.f;
                }
            }

            if (UWorld* World = Char->GetWorld())
            {
                World->GetTimerManager().ClearTimer(EndTimer);
            }
        }
    }

    // close window
    bDashActive = false;
    DashesThisWindow = 0;
    UE_LOG(LogTemp, Warning, TEXT("Dash[Abil]: EndAbility: window closed"));

    SavedBrakingFriction = -1.f;
    SavedFallingLateralFriction = -1.f;
    SavedBrakingDecelFalling = -1.f;
    SavedAirControl = -1.f;

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
