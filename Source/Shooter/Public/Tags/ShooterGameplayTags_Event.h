#pragma once
#include "NativeGameplayTags.h"

namespace ShooterTags
{
    // Arena lifecycle events
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Arena_Start);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Arena_WaveCleared);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Arena_End);

    // Future expansion (optional)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Arena_MorphBegin);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Arena_MorphEnd);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Arena_RewardSpawned);

    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Run_ArenaCleared);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Run_Start);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Run_End);

}
