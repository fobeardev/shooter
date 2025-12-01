#pragma once
#include "NativeGameplayTags.h"

namespace ShooterTags
{
	// Transient combat states
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_IFrame);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dashing);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dashing_Lockout);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Sliding);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_ADS);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Reloading);

	// Status effects
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Bleed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Burn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Shock);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Slow);
}
