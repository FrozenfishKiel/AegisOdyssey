#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "NativeGameplayTags.h"
#include "GameplayTagContainer.h"
#include "AOCombatResultMessage.generated.h"

AEGISODYSSEY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AO_Combat_Message);

UENUM(BlueprintType)
enum class EAOCombatResultType : uint8
{
	// 没有有效战斗结果。
	None UMETA(DisplayName = "None"),

	// 正式生命伤害结果。
	Damage UMETA(DisplayName = "Damage"),

	// 完全格挡成立，本次命中未进入生命伤害尾链。
	Blocked UMETA(DisplayName = "Blocked"),

	// 命中打在无敌帧上，当前方案下视为无效命中。
	Invulnerable UMETA(DisplayName = "Invulnerable"),

	// 弹反成立，命中被改写为弹反交互。
	Parry UMETA(DisplayName = "Parry"),

	// 韧性被打空后的正式破韧结果。
	Broken UMETA(DisplayName = "Broken")
};

UENUM(BlueprintType)
enum class EAOCombatFloatingTextType : uint8
{
	// 不建议显示任何提示字。
	None UMETA(DisplayName = "None"),

	// 普通伤害数字。
	Damage UMETA(DisplayName = "Damage"),

	// 暴击类提示。
	Critical UMETA(DisplayName = "Critical"),

	// 弹反提示。
	Parry UMETA(DisplayName = "Parry"),

	// 破韧提示。
	Broken UMETA(DisplayName = "Broken")
};

// Unified combat result payload.
// Combat truth is finalized first, then broadcast through this message for UI / VM / feedback subscribers.
USTRUCT(BlueprintType)
struct FAOCombatResultMessage
{
	GENERATED_BODY()

	// 统一战斗消息频道。
	// UI / ViewModel / 其他表现系统如果要按消息域过滤，可以先读这个字段。
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	FGameplayTag Verb = TAG_AO_Combat_Message;

	// 这次命中最终被战斗系统解释成哪一种正式结果。
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	EAOCombatResultType ResultType = EAOCombatResultType::None;

	// 结算层给表现层的推荐提示类型。
	// UI 不应该再自己从局部状态反推提示类别。
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	EAOCombatFloatingTextType FloatingTextType = EAOCombatFloatingTextType::None;

	// 是否建议显示提示字。
	// 例如完全格挡、无敌命中这类当前方案下应明确为 false。
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bShouldDisplayFloatingText = false;

	// 以下布尔字段都属于已经完成结算的战斗真相。
	// 表现层只能消费，不能再根据局部状态自行猜测。
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bIsCritical = false;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bWasBlocked = false;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bWasParried = false;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bHitInvulnerability = false;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bTargetBroken = false;

	// 归属对象字段，供表现层做过滤、定位和归因使用。
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	TObjectPtr<AActor> Instigator = nullptr;

	// 本次结果实际作用到谁身上。
	// 例如 Damage / Blocked / Broken 都可以从这里知道受影响目标是谁。
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	TObjectPtr<AActor> Target = nullptr;

	// 这次命中真正的作用来源。
	// 常见情况是武器、投射体、技能生成物，用于表现和溯源时区分“施放者”和“实际命中载体”。
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	TObjectPtr<AActor> EffectCauser = nullptr;

	// 推荐表现入口，不负责改写任何数值真相。
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	FGameplayTag CueTag;

	// 统一攻击来源语义。
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	FGameplayTag AttackTag;

	// 如果这次攻击来自技能，这里记录技能语义标签。
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	FGameplayTag SkillTag;

	// 如果这次攻击带武器语义，这里记录武器标签。
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	FGameplayTag WeaponTag;

	// 伤害类型标签集合。
	// 第一阶段先把它们稳定带进统一消息，后续做抗性、特殊表现、多人同步时继续复用。
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	FGameplayTagContainer DamageTypeTags;

	// 结算后的正式结果值。
	// Damage 结果下主要读 HealthDamage；Parry / Blocked / Broken 更常读后面的韧性和体力字段。
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	float HealthDamage = 0.0f;

	// 本次结果实际扣除的韧性值。
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	float StaminaDamage = 0.0f;

	// 本次结果实际扣除的体力值。
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	float VigorCost = 0.0f;

	// 命中的空间信息，供世界空间表现直接消费。
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	FHitResult HitResult;

	// 调试与日志用的紧凑字符串。
	// 当需要快速确认“这一击最后被解释成了什么”时，优先看这里输出的统一摘要。
	AEGISODYSSEY_API FString ToString() const;
};
