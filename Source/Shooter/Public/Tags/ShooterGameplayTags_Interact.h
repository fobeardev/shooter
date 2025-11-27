#pragma once

#include "NativeGameplayTags.h"

namespace ShooterTags
{
    // Player uses Interact ability
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Interact);

    // Event sent from GA_Interact to the target actor
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Interact);

    // UI hint when interactable is available
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Interact_Available);

    // Optional: mark actors as interactable
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Interactable);
}
