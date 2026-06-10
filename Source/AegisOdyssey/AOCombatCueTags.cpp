#include "AOCombatCueTags.h"

namespace AOCombatCueTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(
		GameplayCue_Combat_Hit,
		"GameplayCue.Combat.Hit",
		"Combat hit gameplay cue");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(
		GameplayCue_Combat_Block,
		"GameplayCue.Combat.Block",
		"Combat block gameplay cue");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(
		GameplayCue_Combat_Parry,
		"GameplayCue.Combat.Parry",
		"Combat parry gameplay cue");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(
		GameplayCue_Combat_Broken,
		"GameplayCue.Combat.Broken",
		"Combat broken gameplay cue");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(
		GameplayCue_Combat_AttackEffects_WeaponSwingLoop,
		"GameplayCue.Combat.AttackEffects.WeaponSwingLoop",
		"Combat attack effect cue for weapon swing loop visuals");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(
		GameplayCue_Combat_AttackEffects_WeaponHitBurst,
		"GameplayCue.Combat.AttackEffects.WeaponHitBurst",
		"Combat attack effect cue for weapon hit burst visuals");
}
