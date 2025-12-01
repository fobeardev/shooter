#pragma once
#include "NativeGameplayTags.h"

namespace ShooterTags
{
	// Movement
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Movement_Dash);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Movement_Slide);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Movement_Grapple);

	// Weapon verbs
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Weapon_Fire);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Weapon_Reload);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Weapon_ADS);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Weapon_Switch);

	// Utility
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Utility_Interact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Utility_UseItem);
}
