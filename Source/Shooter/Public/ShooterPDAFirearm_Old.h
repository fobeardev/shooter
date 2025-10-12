// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/SKGPDAFirearm.h"
#include "GameplayTagContainer.h"
#include "ShooterPDAFirearm_Old.generated.h"

class USoundCue;
class UAnimMontage;
class UNiagaraSystem;
class UAnimInstance;

UCLASS(BlueprintType)
class SHOOTER_API UShooterPDAFirearm_Old : public USKGPDAFirearm
{
	GENERATED_BODY()

public:
	/** -------- Audio -------- */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundCue> Audio_Safety = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundCue> Audio_Fire = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TObjectPtr<USoundCue> Audio_FireSuppressed = nullptr;

	/** -------- Animation (Montages) -------- */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> Animation_Firearm_Reload = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> Animation_Player_Reload = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> Animation_Firearm_Fire = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> Animation_Player_Fire = nullptr;

	/** Upper-body layer Anim BP (class), optional */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> UpperBodyLayer = nullptr;

	/** Main Anim BP class used on the gun’s SkeletalMesh */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TSubclassOf<UAnimInstance> AnimBP = nullptr;

	/** -------- Particles (Niagara) -------- */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Particles")
	TObjectPtr<UNiagaraSystem> NS_EmptyCase = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Particles")
	TObjectPtr<UNiagaraSystem> NS_MuzzleFlash = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Particles")
	TObjectPtr<UNiagaraSystem> NS_MuzzleFlashSuppressed = nullptr;

	/** Spawn a case when firing (like BP boolean) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Particles")
	bool bEjectCaseOnFire = false;

	/** -------- Shooting -------- */
	// Matches BP "FireModes" array of GameplayTags
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooting")
	FGameplayTagContainer FireModes;

	// Matches BP "FireRate" (shots/sec). Your gun converts this to delay.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shooting", meta = (ClampMin = "0.0"))
	float FireRate = 0.f;
};
