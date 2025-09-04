﻿// Copyright 2023, Dakota Dawe, All rights reserved


#include "Components/SKGLightLaserComponent.h"
#include "Subsystems/SKGInfraredWorldSubsystem.h"
#include "DataAssets/Initialize/SKGPDALightLaserInitialize.h"
#include "DeveloperSettings/SKGShooterFrameworkDeveloperSettings.h"

#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

DECLARE_CYCLE_STAT(TEXT("SKGLightLaser Tick"), STAT_SKGLightLaserTick, STATGROUP_SKGLightLaserComponent);

USKGLightLaserComponent::USKGLightLaserComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bWantsInitializeComponent = false;
	SetIsReplicatedByDefault(true);
}

USKGLightLaserComponent* USKGLightLaserComponent::GetLightLaserComponent(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<USKGLightLaserComponent>() : nullptr;
}

void USKGLightLaserComponent::InitializeLightLaserComponent()
{
	InitializeComponentFromData();
	
	if (const USKGShooterFrameworkDeveloperSettings* DeveloperSettings = GetDefault<USKGShooterFrameworkDeveloperSettings>())
	{
		LaserSettings.LaserCollisionChannel = DeveloperSettings->LaserCollisionChannel;
	}
	
	LocalPlayerController = GetWorld()->GetFirstPlayerController();
	SetComponentsAndState();
	RegisterAsInfraredDevice();
}

void USKGLightLaserComponent::BeginPlay()
{
	Super::BeginPlay();
	if (bAutoInitialize)
	{
		InitializeLightLaserComponent();
	}
}

void USKGLightLaserComponent::InitializeComponentFromData()
{
	if (InitializationSettingsDataAsset)
	{
		if (!bOverrideComponentNames)
		{
			LightComponentName = InitializationSettingsDataAsset->LightComponentName;
			LaserMeshComponentName = InitializationSettingsDataAsset->LaserMeshComponentName;
			LaserDotComponentName = InitializationSettingsDataAsset->LaserDotComponentName;
		}
		bHasInfraredMode = InitializationSettingsDataAsset->bHasInfraredMode;
		bAutoCallForceNetUpdate = InitializationSettingsDataAsset->bAutoCallForceNetUpdate;
		GameplayTags = InitializationSettingsDataAsset->GameplayTags;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializationSettingsDataAsset NOT set on Actor: %s on Component: %s Please utilize this data asset as it will be mandatory moving forwards"), *GetOwner()->GetName(), *GetName());
	}
	
	if (LightLaserSettingsDataAsset)
	{
		LaserSettings = LightLaserSettingsDataAsset->LaserSettings;
		LightSettings = LightLaserSettingsDataAsset->LightSettings;
		LightLaserCycleModes = LightLaserSettingsDataAsset->LightLaserCycleModes;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SKGLightLaserComponent on Actor: %s is NOT using the LightLaserSettingsDataAsset for configuration. Please switch to the new system before updating as it will be mandatory in a future version"), *GetOwner()->GetName());
	}
}

void USKGLightLaserComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	UnregisterAsInfraredDevice();
}

void USKGLightLaserComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.RepNotifyCondition = REPNOTIFY_Always;
	DOREPLIFETIME_WITH_PARAMS_FAST(USKGLightLaserComponent, LaserState, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(USKGLightLaserComponent, bDeviceInfraredOn, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(USKGLightLaserComponent, LightState, Params);
}

void USKGLightLaserComponent::TryForceNetUpdate() const
{
	if (bAutoCallForceNetUpdate)
	{
		GetOwner()->ForceNetUpdate();
	}
}

void USKGLightLaserComponent::SetComponentsAndState()
{
	// THIS NEEDS REIMPLEMENTED
	ensureAlwaysMsgf(LightComponentName != NAME_None || LaserMeshComponentName != NAME_None, TEXT("Light Component Name OR Laser Mesh Component Name must be valid on Actor: %s in the SKGLightLaserComponent: %s"), *GetOwner()->GetName(), *GetName());

	TInlineComponentArray<UActorComponent*> Components(GetOwner());
	for (UActorComponent* Component : Components)
	{
		if (Component)
		{
			if (Component->GetFName().IsEqual(LightComponentName))
			{
				LightSource = Cast<ULightComponent>(Component);
			}
			else if (Component->GetFName().IsEqual(LaserMeshComponentName))
			{
				LaserMesh = Cast<UStaticMeshComponent>(Component);
			}
			else if (Component->GetFName().IsEqual(LaserDotComponentName))
			{
				LaserDot = Cast<UStaticMeshComponent>(Component);
			}
		}
	}

	// NEEDS TO BE REDONE
	ensureAlwaysMsgf(LightSource || LaserMesh, TEXT("Light Source AND Laser Mesh NOT found on Actor: %s in the SKGLightLaserComponent: %s. Ensure Component Names are correct"), *GetOwner()->GetName(), *GetName());

	if (LightSource)
	{
		OnRep_LightState();
	}
	if (LaserMesh)
	{
		LaserMesh->SetMaterial(0, LaserSettings.DefaultLaserMaterial.Laser);
		LaserDot->SetMaterial(0, LaserSettings.DefaultLaserMaterial.LaserDot);
		LaserState.CurrentLaserMaterial = LaserSettings.DefaultLaserMaterial;
		LaserSettings.SetupDefaultMaterial(LaserSettings.DefaultLaserMaterial);
		OnRep_LaserState();
	}
}

void USKGLightLaserComponent::RegisterAsInfraredDevice()
{
	if (bHasInfraredMode)
	{
		if (USKGInfraredWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<USKGInfraredWorldSubsystem>())
		{
			Subsystem->RegisterInfraredDevice(this);
		}
	}
}

void USKGLightLaserComponent::UnregisterAsInfraredDevice()
{
	if (bHasInfraredMode)
	{
		GetWorld()->GetSubsystem<USKGInfraredWorldSubsystem>()->UnregisterInfraredDevice(this);
	}
}

void USKGLightLaserComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SCOPE_CYCLE_COUNTER(STAT_SKGLightLaserTick);
	
	if (LaserState.bCanScaleLaser && LaserState.LaserMode == ESKGLaserMode::On && LaserMesh)
	{
		PerformLaserScaling();
	}
	if (LightState.bCanStrobe && LightState.LightMode == ESKGLightMode::Strobe && LightSource)
	{
		PerformLightStrobing();
	}
}

void USKGLightLaserComponent::PerformLaserScaling()
{
	const FVector Start = LaserMesh->GetComponentLocation();
	const FVector End = Start + LaserMesh->GetRightVector() * LaserSettings.MaxLaserDistance;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	float LaserScaleLength = LaserSettings.MaxLaserDistance;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, LaserSettings.LaserCollisionChannel, Params))
	{
		OnLaserImpact.Broadcast(HitResult, true);
		LaserScaleLength = HitResult.Distance / LaserSettings.LaserScaleFactor;
		if (LaserScale.Y != LaserScaleLength)
		{
			LaserScale.Y = LaserScaleLength;
			LaserMesh->SetWorldScale3D(LaserScale);
			
			if (!LaserDot->IsVisible())
			{
				LaserDot->SetVisibility(true);
			}
			LaserDot->SetWorldLocation(HitResult.Location);
		}
	}
	else
	{
		OnLaserImpact.Broadcast(HitResult, false);
		LaserScaleLength = LaserSettings.MaxLaserDistance / LaserSettings.LaserScaleFactor;
		LaserScale.Y = LaserScaleLength;
		LaserMesh->SetWorldScale3D(LaserScale);
		
		if (LaserDot && LaserDot->IsVisible())
		{
			LaserDot->SetVisibility(false);
		}
	}
}

void USKGLightLaserComponent::PerformLightStrobing()
{
	const double CurrentStrobeTimeStamp = GetWorld()->GetTimeSeconds();
	if (CurrentStrobeTimeStamp - PreviousStrobeTimeStamp > LightSettings.LightStrobeInterval)
	{
		PreviousStrobeTimeStamp = CurrentStrobeTimeStamp;

		if (LightSource->Intensity == 0.0f)
		{
			LightSource->SetIntensity(LightState.OnIntensity);
			if (OnLightStrobed.IsBound())
			{
				OnLightStrobed.Broadcast(true);
			}
		}
		else
		{
			LightSource->SetIntensity(0.0f);
			if (OnLightStrobed.IsBound())
			{
				OnLightStrobed.Broadcast(false);
			}
		}
	}
}

void USKGLightLaserComponent::OnRep_DeviceInfraredOn()
{
	OnRep_LaserState();
	OnRep_LightState();
	if (OnInfraredModeChanged.IsBound())
	{
		OnInfraredModeChanged.Broadcast(bDeviceInfraredOn);
	}
}

bool USKGLightLaserComponent::Server_SetInfraredMode_Validate(bool bInfraredOn)
{
	return bHasInfraredMode;
}

void USKGLightLaserComponent::Server_SetInfraredMode_Implementation(bool bInfraredOn)
{
	bDeviceInfraredOn = bInfraredOn;
	MARK_PROPERTY_DIRTY_FROM_NAME(USKGLightLaserComponent, bDeviceInfraredOn, this);
	OnRep_DeviceInfraredOn();
	TryForceNetUpdate();
}

void USKGLightLaserComponent::SetInfraredMode(bool InfraredModeOn)
{
	if (bHasInfraredMode && bDeviceInfraredOn != InfraredModeOn)
	{
		bDeviceInfraredOn = InfraredModeOn;
		if (HasAuthority())
		{
			MARK_PROPERTY_DIRTY_FROM_NAME(USKGLightLaserComponent, bDeviceInfraredOn, this);
			TryForceNetUpdate();
		}
		else
		{
			Server_SetInfraredMode(bDeviceInfraredOn);
		}
		OnRep_DeviceInfraredOn();
	}
}

void USKGLightLaserComponent::OnRep_LaserState()
{
	if (LaserMesh)
	{
		bool bSetLaserVisible = LaserState.LaserMode == ESKGLaserMode::On;
		const bool bPlayerInfraredOn = GetWorld()->GetSubsystem<USKGInfraredWorldSubsystem>()->IsInfraredModeEnabledOnPlayer();
		if (bDeviceInfraredOn && !bPlayerInfraredOn)
		{
			if (LaserState.LaserMode == ESKGLaserMode::On)
			{
				bSetLaserVisible = false;
			}
		}

		const FSKGLaserMaterial NewLaserMaterial = LaserSettings.GetMaterial(bDeviceInfraredOn);
		if (LaserState.CurrentLaserMaterial != NewLaserMaterial)
		{
			LaserMesh->SetMaterial(0, NewLaserMaterial.Laser);
			LaserDot->SetMaterial(0, NewLaserMaterial.LaserDot);
			LaserState.CurrentLaserMaterial = NewLaserMaterial;
		}
		
		LaserMesh->SetVisibility(bSetLaserVisible);
		LaserDot->SetVisibility(bSetLaserVisible);
		LaserState.bCanScaleLaser = bSetLaserVisible;

		if (bSetLaserVisible)
		{
			if (!IsComponentTickEnabled())
			{
				SetComponentTickEnabled(true);
			}
		}
		else if (LightState.LightMode != ESKGLightMode::Strobe && IsComponentTickEnabled())
		{
			SetComponentTickEnabled(false);
		}
	}
	if (OnLaserStateChanged.IsBound())
	{
		OnLaserStateChanged.Broadcast(LaserState.LaserMode);
	}
}

bool USKGLightLaserComponent::Server_SetLaserMode_Validate(ESKGLaserMode LaserMode)
{
	return true;
}

void USKGLightLaserComponent::Server_SetLaserMode_Implementation(ESKGLaserMode LaserMode)
{
	LaserState.LaserMode = LaserMode;
	MARK_PROPERTY_DIRTY_FROM_NAME(USKGLightLaserComponent, LaserState, this);
	OnRep_LaserState();
	TryForceNetUpdate();
}

void USKGLightLaserComponent::SetLaserMode(ESKGLaserMode LaserMode)
{
	if (LaserMesh && LaserState.LaserMode != LaserMode)
	{
		LaserState.LaserMode = LaserMode;
		if (HasAuthority())
		{
			MARK_PROPERTY_DIRTY_FROM_NAME(USKGLightLaserComponent, LaserState, this);
			TryForceNetUpdate();
		}
		else
		{
			Server_SetLaserMode(LaserState.LaserMode);
		}
		OnRep_LaserState();
	}
}

void USKGLightLaserComponent::OnRep_LightState()
{
	if (LightSource)
	{
		bool bSetLightVisible = LightState.LightMode == ESKGLightMode::On || LightState.LightMode == ESKGLightMode::Strobe;
		const bool bPlayerInfraredOn = GetWorld()->GetSubsystem<USKGInfraredWorldSubsystem>()->IsInfraredModeEnabledOnPlayer();
		if (bDeviceInfraredOn && !bPlayerInfraredOn)
		{
			bSetLightVisible = false;
		}

		LightState.OnIntensity = bSetLightVisible ? LightSettings.GetLightIntensity(bDeviceInfraredOn, LightState.LightIntensityIndex) : 0.0f;
		LightSource->SetIntensity(LightState.OnIntensity);
		LightState.bCanStrobe = bSetLightVisible;
		if (bSetLightVisible)
		{
			if (LightState.LightMode == ESKGLightMode::Strobe && !IsComponentTickEnabled())
			{
				SetComponentTickEnabled(true);
			}
		}
		else if ((LaserState.LaserMode == ESKGLaserMode::Off || (LaserMesh && !LaserMesh->IsVisible())) && IsComponentTickEnabled())
		{
			SetComponentTickEnabled(false);
		}
	}
	if (OnLightModeChanged.IsBound())
	{
		OnLightModeChanged.Broadcast(LightState.LightMode);
	}
}

bool USKGLightLaserComponent::Server_SetLightMode_Validate(ESKGLightMode LightMode)
{
	return true;
}

void USKGLightLaserComponent::Server_SetLightMode_Implementation(ESKGLightMode LightMode)
{
	LightState.LightMode = LightMode;
	MARK_PROPERTY_DIRTY_FROM_NAME(USKGLightLaserComponent, LightState, this);
	OnRep_LightState();
	TryForceNetUpdate();
}

void USKGLightLaserComponent::SetLightMode(ESKGLightMode LightMode)
{
	if (LightSource && LightState.LightMode != LightMode)
	{
		LightState.LightMode = LightMode;
		if (HasAuthority())
		{
			MARK_PROPERTY_DIRTY_FROM_NAME(USKGLightLaserComponent, LightState, this);
			TryForceNetUpdate();
		}
		else
		{
			Server_SetLightMode(LightState.LightMode);
		}
		OnRep_LightState();
	}
}

bool USKGLightLaserComponent::Server_SetLightIntensityIndex_Validate(int32 Index)
{
	return true;
}

void USKGLightLaserComponent::Server_SetLightIntensityIndex_Implementation(int32 Index)
{
	LightState.LightIntensityIndex = Index;
	MARK_PROPERTY_DIRTY_FROM_NAME(USKGLightLaserComponent, LightState, this);
	OnRep_LightState();
	TryForceNetUpdate();
}

void USKGLightLaserComponent::SetLightIntensityIndex(int32 Index)
{
	if (LightSource && LightState.LightIntensityIndex != Index && LightSettings.IsValidIndex(bDeviceInfraredOn, Index))
	{
		LightState.LightIntensityIndex = Index;
		if (HasAuthority())
		{
			MARK_PROPERTY_DIRTY_FROM_NAME(USKGLightLaserComponent, LightState, this);
			TryForceNetUpdate();
		}
		else
		{
			Server_SetLightIntensityIndex(Index);
		}
		OnRep_LightState();
	}
}

bool USKGLightLaserComponent::Server_SetLightLaserMode_Validate(const FSKGLightLaserCycleMode& LightLaserMode)
{	// prevent trying to pass in a mode that may not exist on this device
	for (const FSKGLightLaserCycleMode& LightLaserCycleMode : LightLaserCycleModes.LightLaserModes)
	{
		if (LightLaserCycleMode == LightLaserMode)
		{
			return true;
		}
	}
	return false;
}

void USKGLightLaserComponent::Server_SetLightLaserMode_Implementation(const FSKGLightLaserCycleMode& LightLaserMode)
{
	SetLaserMode(LightLaserMode.LaserMode);
	SetLightMode(LightLaserMode.LightMode);
	SetInfraredMode(LightLaserMode.bInfrared);
}

void USKGLightLaserComponent::SetLightLaserMode(const FSKGLightLaserCycleMode& LightLaserMode)
{
	if (HasAuthority())
	{
		SetLaserMode(LightLaserMode.LaserMode);
		SetLightMode(LightLaserMode.LightMode);
		SetInfraredMode(LightLaserMode.bInfrared);
	}
	else
	{
		Server_SetLightLaserMode(LightLaserMode);
		if (LightSource && LightState.LightMode != LightLaserMode.LightMode)
		{
			LightState.LightMode = LightLaserMode.LightMode;
		}
		if (LaserMesh && LaserState.LaserMode != LightLaserMode.LaserMode)
		{
			LaserState.LaserMode = LightLaserMode.LaserMode;
		}
		if (bHasInfraredMode)
		{
			bDeviceInfraredOn = LightLaserMode.bInfrared;
		}
		
		OnRep_LaserState();
		OnRep_LightState();
		OnRep_DeviceInfraredOn();
	}
}

void USKGLightLaserComponent::CycleLightLaserMode()
{
	SetLightLaserMode(LightLaserCycleModes.GetNextLightLaserMode());
}

bool USKGLightLaserComponent::SetLightLaserCycleModeIndex(const int32 Index)
{
	FSKGLightLaserCycleMode LightLaserCycleMode;
	const bool bValidMode = LightLaserCycleModes.GetLightLaserModeAtIndex(Index, LightLaserCycleMode);
	if (bValidMode)
	{
		SetLightLaserMode(LightLaserCycleMode);
	}
	return bValidMode;
}

void USKGLightLaserComponent::OnInfraredEnabledForPlayer()
{
	OnRep_DeviceInfraredOn();
}

void USKGLightLaserComponent::OnInfraredDisabledForPlayer()
{
	OnRep_DeviceInfraredOn();
}
