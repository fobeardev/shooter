#pragma once

/*
 * Projectile identity & behavior tags.
 *
 * These tags describe an individual projectile instance,
 * NOT player capabilities or weapon properties.
 */

namespace ShooterTags
{
    // -----------------------------
    // Projectile Type
    // -----------------------------
    SHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile_Type_Bullet);
    SHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile_Type_Explosive);
    SHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile_Type_Energy);

    // -----------------------------
    // Projectile Element
    // -----------------------------
    SHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile_Element_None);
    SHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile_Element_Fire);
    SHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile_Element_Shock);
    SHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile_Element_Corrosive);

    // -----------------------------
    // Projectile Pattern
    // -----------------------------
    SHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile_Pattern_Single);
    SHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile_Pattern_Spread);
    SHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile_Pattern_Burst);

    // -----------------------------
    // Projectile Modifiers
    // -----------------------------
    SHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile_Modifier_Ricochet);
    SHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile_Modifier_Pierce);
    SHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile_Modifier_Homing);
    SHOOTER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile_Modifier_Accelerate);
}
