#include "Gameplay/Tags/ShooterGameplayTags.h"
#include "Gameplay/Tags/ShooterGameplayTags_Ability.h"
#include "Gameplay/Tags/ShooterGameplayTags_State.h"
#include "Gameplay/Tags/ShooterGameplayTags_Cue.h"
#include "Gameplay/Tags/ShooterGameplayTags_Data.h"
#include "Gameplay/Tags/ShooterGameplayTags_Weapon.h"
#include "Gameplay/Tags/ShooterGameplayTags_Attachment.h"
#include "Gameplay/Tags/ShooterGameplayTags_Augment.h"
#include "Gameplay/Tags/ShooterGameplayTags_Meta.h"
#include "Gameplay/Tags/ShooterGameplayTags_Interact.h"

// Ability
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Ability_Movement_Dash, "Ability.Movement.Dash");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Ability_Movement_Slide, "Ability.Movement.Slide");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Ability_Movement_Grapple, "Ability.Movement.Grapple");

UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Ability_Weapon_Fire, "Ability.Weapon.Fire");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Ability_Weapon_Reload, "Ability.Weapon.Reload");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Ability_Weapon_ADS, "Ability.Weapon.ADS");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Ability_Weapon_Switch, "Ability.Weapon.Switch");

UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Ability_Utility_Interact, "Ability.Utility.Interact");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Ability_Utility_UseItem, "Ability.Utility.UseItem");

// UI 
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::UI_Interact_Available, "UI.Interact.Available");

// State
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::State_IFrame, "State.IFrame");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::State_Dashing, "State.Dashing");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::State_Dashing_Lockout, "State.Dashing.Lockout");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::State_Sliding, "State.Sliding");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::State_ADS, "State.ADS");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::State_Reloading, "State.Reloading");

UE_DEFINE_GAMEPLAY_TAG(ShooterTags::State_Bleed, "State.Bleed");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::State_Burn, "State.Burn");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::State_Shock, "State.Shock");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::State_Slow, "State.Slow");

UE_DEFINE_GAMEPLAY_TAG(ShooterTags::State_Interactable, "State.Interactable");

// Enemy
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Enemy_Type_Charger, "Enemy.Type.Charger");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Enemy_Type_Marksman, "Enemy.Type.Marksman");

// GameplayCues (must start with GameplayCue.)
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::GameplayCue_Dash_Start, "GameplayCue.Dash.Start");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::GameplayCue_Dash_End, "GameplayCue.Dash.End");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::GameplayCue_Weapon_Muzzle, "GameplayCue.Weapon.Muzzle");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::GameplayCue_Weapon_Reload, "GameplayCue.Weapon.Reload");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::GameplayCue_Damage_Hit, "GameplayCue.Damage.Hit");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::GameplayCue_Damage_Death, "GameplayCue.Damage.Death");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::GameplayCue_Enemy_Flank, "GameplayCue.Enemy.Flank");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::GameplayCue_Enemy_Retreat, "GameplayCue.Enemy.Retreat");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::GameplayCue_Combat_MeleeHit, "GameplayCue.Combat.MeleeHit");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::GameplayCue_Clone_Reprint, "GameplayCue.Clone.Reprint");

// Data
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Data_Health_Current, "Data.Health.Current");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Data_Health_Max, "Data.Health.Max");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Data_Stamina_Current, "Data.Stamina.Current");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Data_Stamina_Max, "Data.Stamina.Max");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Data_Movement_Speed, "Data.Movement.Speed");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Data_Weapon_DamageScalar, "Data.Weapon.DamageScalar");

UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Damage_Ballistic, "Damage.Ballistic");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Damage_Explosive, "Damage.Explosive");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Damage_Energy, "Damage.Energy");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Damage_Corrosive, "Damage.Corrosive");

// Weapon
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Class_Pistol, "Weapon.Class.Pistol");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Class_SMG, "Weapon.Class.SMG");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Class_Shotgun, "Weapon.Class.Shotgun");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Class_AR, "Weapon.Class.AR");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Class_DMR, "Weapon.Class.DMR");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Class_Sniper, "Weapon.Class.Sniper");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Class_LMG, "Weapon.Class.LMG");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Class_Rocket, "Weapon.Class.Rocket");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Class_Railgun, "Weapon.Class.Railgun");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Class_Sword, "Weapon.Class.Sword");

UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_FireMode_Semi, "Weapon.FireMode.Semi");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_FireMode_Burst, "Weapon.FireMode.Burst");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_FireMode_Auto, "Weapon.FireMode.Auto");

UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Spread_Ricochet, "Weapon.Spread.Ricochet");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Projectile_Pierce, "Weapon.Projectile.Pierce");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Projectile_Homing, "Weapon.Projectile.Homing");

UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Ammo_Light, "Weapon.Ammo.Light");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Ammo_Medium, "Weapon.Ammo.Medium");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Ammo_Heavy, "Weapon.Ammo.Heavy");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Ammo_Rocket, "Weapon.Ammo.Rocket");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Weapon_Ammo_Energy, "Weapon.Ammo.Energy");

// Attachment
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Attachment_Slot_Muzzle, "Attachment.Slot.Muzzle");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Attachment_Slot_Optic, "Attachment.Slot.Optic");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Attachment_Slot_Underbarrel, "Attachment.Slot.Underbarrel");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Attachment_Slot_Side, "Attachment.Slot.Side");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Attachment_Slot_Stock, "Attachment.Slot.Stock");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Attachment_Slot_Mag, "Attachment.Slot.Mag");

UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Attachment_Mod_RecoilDown, "Attachment.Mod.RecoilDown");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Attachment_Mod_SpreadDown, "Attachment.Mod.SpreadDown");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Attachment_Mod_DamageUp, "Attachment.Mod.DamageUp");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Attachment_Mod_ProjectileSpeedUp, "Attachment.Mod.ProjectileSpeedUp");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Attachment_Mod_ReloadSpeedUp, "Attachment.Mod.ReloadSpeedUp");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Attachment_Mod_ADSFaster, "Attachment.Mod.ADSFaster");

// Augments
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Augment_Projectile_Ricochet, "Augment.Projectile.Ricochet");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Augment_Projectile_SplitShot, "Augment.Projectile.SplitShot");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Augment_Projectile_Homing, "Augment.Projectile.Homing");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Augment_Projectile_Accelerate, "Augment.Projectile.Accelerate");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Augment_Projectile_Pierce, "Augment.Projectile.Pierce");

UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Augment_Mobility_DashIFRamesUp, "Augment.Mobility.DashIFRamesUp");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Augment_Mobility_DashChargesUp, "Augment.Mobility.DashChargesUp");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Augment_Sustain_LifeOnKill, "Augment.Sustain.LifeOnKill");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Augment_Economy_LootUp, "Augment.Economy.LootUp");

// Meta
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Meta_Genome_HealthTier, "Meta.Genome.HealthTier");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Meta_Genome_DashTier, "Meta.Genome.DashTier");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Meta_Genome_WeaponUnlocks, "Meta.Genome.WeaponUnlocks");

UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Meta_Run_RoomClears, "Meta.Run.RoomClears");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Meta_Run_Killstreak, "Meta.Run.Killstreak");

// Arena Events
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Event_Arena_Start, "Event.Arena.Start");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Event_Arena_WaveCleared, "Event.Arena.WaveCleared");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Event_Arena_End, "Event.Arena.End");

// Optional extended arena flow
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Event_Arena_MorphBegin, "Event.Arena.MorphBegin");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Event_Arena_MorphEnd, "Event.Arena.MorphEnd");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Event_Arena_RewardSpawned, "Event.Arena.RewardSpawned");

// Run Events
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Event_Run_ArenaCleared, "Event.Run.ArenaCleared");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Event_Run_Start, "Event.Run.Start");
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Event_Run_End, "Event.Run.End");

// Interaction Events
UE_DEFINE_GAMEPLAY_TAG(ShooterTags::Event_Interact, "Event.Interact");
