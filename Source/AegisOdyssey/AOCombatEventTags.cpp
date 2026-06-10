#include "AOCombatEventTags.h"

namespace AOCombatEventTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(
		GameplayEvent_Combat_HitReact_Activate,
		"GameplayEvent.Combat.HitReact.Activate",
		"Activate hit react ability");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(
		GameplayEvent_Combat_HitReact_AllowMove,
		"GameplayEvent.Combat.HitReact.AllowMove",
		"Open hit react movement recovery window");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(
		GameplayEvent_Combat_HitReact_Finish,
		"GameplayEvent.Combat.HitReact.Finish",
		"Finish hit react ability");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(
		GameplayEvent_Combat_ParriedReact_Activate,
		"GameplayEvent.Combat.ParriedReact.Activate",
		"Activate parried react ability");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(
		GameplayEvent_Combat_ParriedReact_AllowMove,
		"GameplayEvent.Combat.ParriedReact.AllowMove",
		"Open parried react movement recovery window");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(
		GameplayEvent_Combat_ParriedReact_Finish,
		"GameplayEvent.Combat.ParriedReact.Finish",
		"Finish parried react ability");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(
		GameplayEvent_Combat_Block_ParrySuccess,
		"GameplayEvent.Combat.Block.ParrySuccess",
		"Notify active block ability that a parry resolved");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(
		GameplayEvent_Combat_Block_FullBlockSuccess,
		"GameplayEvent.Combat.Block.FullBlockSuccess",
		"Notify active block ability that a full block resolved");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(
		GameplayEvent_Combat_Block_PartialBlockSuccess,
		"GameplayEvent.Combat.Block.PartialBlockSuccess",
		"Notify active block ability that a partial block resolved");
}
