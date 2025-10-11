#pragma once
#include "NativeGameplayTags.h"

namespace ShooterTags
{
	// Projectile augments
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Augment_Projectile_Ricochet);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Augment_Projectile_SplitShot);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Augment_Projectile_Homing);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Augment_Projectile_Accelerate);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Augment_Projectile_Pierce);

	// Mobility augments
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Augment_Mobility_DashIFRamesUp);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Augment_Mobility_DashChargesUp);

	// Sustain / Economy
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Augment_Sustain_LifeOnKill);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Augment_Economy_LootUp);
}
