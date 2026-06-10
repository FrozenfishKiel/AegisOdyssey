#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GameplayTagContainer.h"
#include "AOCombatGameplayCueNotify_Burst.generated.h"

class UNiagaraSystem;
class USoundBase;
struct FGameplayCueParameters;

UENUM(BlueprintType)
enum class EAOCombatCueBoolRequirement : uint8
{
	Ignore UMETA(DisplayName = "Ignore"),
	MustBeTrue UMETA(DisplayName = "Must Be True"),
	MustBeFalse UMETA(DisplayName = "Must Be False")
};

USTRUCT(BlueprintType)
struct FAOCombatCueBurstEffectEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	TObjectPtr<UNiagaraSystem> NiagaraEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	TObjectPtr<USoundBase> Sound = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	FVector Scale = FVector(1.0f);
};

USTRUCT(BlueprintType)
struct FAOCombatCueBurstEffectGroup
{
	GENERATED_BODY()

	// 一组效果是否生效，先看这些捕获到的 SourceTags。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue|Match")
	FGameplayTagContainer RequiredSourceTags;

	// 一组效果是否生效，再看这些捕获到的 TargetTags。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue|Match")
	FGameplayTagContainer RequiredTargetTags;

	// 战斗真相里如果带了 AttackTag，这里可以继续细分到具体攻击语义。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue|Match")
	FGameplayTagContainer RequiredAttackTags;

	// 技能来源命中想在同一个 GC 里分流时，直接用 SkillTag 过滤。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue|Match")
	FGameplayTagContainer RequiredSkillTags;

	// 同一个命中 GC 里需要区分剑、枪等武器来源时，走 WeaponTag 过滤。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue|Match")
	FGameplayTagContainer RequiredWeaponTags;

	// 元素或伤害类型附加层继续留在同一个 GC 内部分流。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue|Match")
	FGameplayTagContainer RequiredDamageTypeTags;

	// 暴击、格挡、弹反这类结果真相不是标签，但同样是常见的 GC 内部分流条件。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue|Match")
	EAOCombatCueBoolRequirement CriticalRequirement = EAOCombatCueBoolRequirement::Ignore;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue|Match")
	EAOCombatCueBoolRequirement BlockedRequirement = EAOCombatCueBoolRequirement::Ignore;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue|Match")
	EAOCombatCueBoolRequirement ParriedRequirement = EAOCombatCueBoolRequirement::Ignore;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue|Match")
	EAOCombatCueBoolRequirement HitInvulnerabilityRequirement = EAOCombatCueBoolRequirement::Ignore;

	// 条件命中后，这一组效果里的 Niagara / 音效都会依次执行。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	TArray<FAOCombatCueBurstEffectEntry> EffectEntries;
};

// Combat 专用瞬时 GameplayCue。
// 这层允许“一个 GC 里挂多组效果，并按战斗标签 / 真相继续分流”，
// 这样 Profile 只负责决定触发哪个 GC，GC 自己再决定内部播哪几组效果。
UCLASS(Blueprintable)
class AEGISODYSSEY_API UAOCombatGameplayCueNotify_Burst : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

protected:
	// 默认允许多组同时命中并叠加播放；如果想做“命中第一组就停”，可以在资产里显式改掉。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	bool bStopAfterFirstMatchingGroup = false;

	// 一个 GC 内部允许配多组效果，每组自己声明过滤条件。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue")
	TArray<FAOCombatCueBurstEffectGroup> EffectGroups;

private:
	static bool PlayEffectGroup(const UWorld* World, const FGameplayCueParameters& Parameters, const FAOCombatCueBurstEffectGroup& EffectGroup);
	static bool MatchesEffectGroup(const FAOCombatCueBurstEffectGroup& EffectGroup, const FGameplayCueParameters& Parameters);
	static bool MatchesBoolRequirement(EAOCombatCueBoolRequirement Requirement, bool bActualValue);
	static bool MatchesRequiredTags(const FGameplayTagContainer& RequiredTags, const FGameplayTagContainer& ActualTags);
};
