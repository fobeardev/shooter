#pragma once
#include "NativeGameplayTags.h"

namespace ShooterTags
{
	// GameplayCues must start with GameplayCue.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Dash_Start);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Dash_End);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Muzzle);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Reload);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Damage_Hit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Damage_Death);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Enemy_Flank);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Enemy_Retreat);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Combat_MeleeHit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Clone_Reprint);
}
