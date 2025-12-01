#pragma once
#include "NativeGameplayTags.h"

namespace ShooterTags
{
	// Attributes / data keys
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Health_Current);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Health_Max);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stamina_Current);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stamina_Max);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Movement_Speed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Weapon_DamageScalar);

	// Damage types (optional, for resistances/bonuses)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Ballistic);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Explosive);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Energy);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Corrosive);
}
