#include "AOCombatResultMessage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCombatResultMessage)

UE_DEFINE_GAMEPLAY_TAG(TAG_AO_Combat_Message, "AegisOdyssey.Combat.Message");

FString FAOCombatResultMessage::ToString() const
{
	// 统一调试摘要。
	// 这里尽量把“是谁打谁、被解释成什么结果、扣了哪些资源、挂了什么标签”压成一行，
	// 方便在日志里快速定位命中解释是否正确。
	return FString::Printf(
		TEXT("CombatResult Type=%s Instigator=%s Target=%s AttackTag=%s SkillTag=%s WeaponTag=%s HealthDamage=%.2f StaminaDamage=%.2f VigorCost=%.2f Critical=%s Blocked=%s Parried=%s Invuln=%s Broken=%s Cue=%s"),
		*StaticEnum<EAOCombatResultType>()->GetNameStringByValue(static_cast<int64>(ResultType)),
		*GetNameSafe(Instigator),
		*GetNameSafe(Target),
		*AttackTag.ToString(),
		*SkillTag.ToString(),
		*WeaponTag.ToString(),
		HealthDamage,
		StaminaDamage,
		VigorCost,
		bIsCritical ? TEXT("true") : TEXT("false"),
		bWasBlocked ? TEXT("true") : TEXT("false"),
		bWasParried ? TEXT("true") : TEXT("false"),
		bHitInvulnerability ? TEXT("true") : TEXT("false"),
		bTargetBroken ? TEXT("true") : TEXT("false"),
		*CueTag.ToString());
}
