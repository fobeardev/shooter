#pragma once
#include "NativeGameplayTags.h"

namespace ShooterTags
{
	// Permanent progression
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Meta_Genome_HealthTier);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Meta_Genome_DashTier);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Meta_Genome_WeaponUnlocks);

	// Run stats
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Meta_Run_RoomClears);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Meta_Run_Killstreak);
}
