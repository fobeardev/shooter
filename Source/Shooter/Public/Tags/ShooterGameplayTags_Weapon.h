#pragma once
#include "NativeGameplayTags.h"

namespace ShooterTags
{
	// Weapon families
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Class_Pistol);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Class_SMG);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Class_Shotgun);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Class_AR);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Class_DMR);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Class_Sniper);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Class_LMG);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Class_Rocket);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Class_Railgun);

	// Fire modes
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_FireMode_Semi);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_FireMode_Burst);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_FireMode_Auto);

	// Projectile behaviors
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Spread_Ricochet);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Projectile_Pierce);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Projectile_Homing);

	// Ammo types
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Ammo_Light);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Ammo_Medium);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Ammo_Heavy);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Ammo_Rocket);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Ammo_Energy);
}
