// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/Abilities/AOGameplayAbility.h"
#include "AegisOdyssey/Character/AOCombatHitPolicy.h"
#include "AOSkillGameplayAbility.generated.h"

class AActor;
class UGameplayEffect;
struct FGameplayCueParameters;
class UAOSkillDefinition;
class UAOSkillExecutionDefinition;
class UAOSkillInstance;

/**
 * 技能系统专用 GameplayAbility 基类。
 *
 * 这一层负责把“技能实例语义”和 GAS 能力主链接起来，
 * 并为具体技能提供统一的执行对象读取入口。
 *
 * 注意：
 * 这里故意不再内建“播放动画 -> 等事件 -> 开始执行”的默认流程。
 * 那种流程属于具体技能自己的释放语义，不应该由父类替它做决定。
 */
UCLASS(Abstract)
class AEGISODYSSEY_API UAOSkillGameplayAbility : public UAOGameplayAbility
{
	GENERATED_BODY()

public:
	UAOSkillGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	UFUNCTION(BlueprintCallable, Category = "AO|Skill Ability")
	UAOSkillInstance* GetSkillInstanceFromCurrentSourceObject() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Skill Ability")
	UAOSkillDefinition* GetSkillDefinitionFromCurrentSourceObject() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Skill Ability")
	UAOSkillExecutionDefinition* GetSkillExecutionDefinitionFromCurrentSourceObject() const;

	// 供技能自己生成出来的运行时对象回调。
	// 例如投射体、持续区域 Actor 等，把命中目标重新送回技能 Ability 的既有结算尾链。
	void RouteSkillEffectApplicationFromRuntimeActor(
		const TArray<AActor*>& Targets,
		const FHitResult* HitResult = nullptr,
		AActor* EffectCauser = nullptr,
		FName SegmentKey = NAME_None) const;

	// 供技能运行时对象查询调试开关。
	// 运行时对象不需要知道内部 CVar 细节，只需要问当前 SkillAbility “能不能画”。
	bool CanRuntimeActorDrawSkillDebug() const;

	// 第二阶段统一的分段键生成入口，运行时对象不再手写各自的字符串规则。
	FName BuildRuntimeHitSegmentKey(FName ExplicitSegmentKey = NAME_None, int32 SegmentIndex = INDEX_NONE) const;

	// 统一从执行对象触发技能自己的瞬时 GameplayCue。
	void ExecuteSkillCue(const FGameplayTag& CueTag, const FGameplayCueParameters& CueParameters) const;

	// 按当前位置/法线快速构造一次 GameplayCue 参数。
	FGameplayCueParameters BuildSkillCueParameters(const FVector& Location, const FVector& Normal, AActor* EffectCauser = nullptr) const;

protected:
	// 这几个函数是 SkillInstance / SkillDefinition / ExecutionDefinition 的统一读取入口。
	// 目的就是把“技能实例语义”稳定限制在 Skill Ability 这一层，不往上反渗透。
	UAOSkillInstance* GetSkillInstanceFromSourceObject(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;
	UAOSkillDefinition* ResolveSkillDefinition(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;
	UAOSkillExecutionDefinition* ResolveSkillExecutionDefinition(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;
	void BuildCooldownIdentityTags(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer& OutTags) const;
	float GetCooldownDurationSeconds(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;
	TSubclassOf<UGameplayEffect> ResolveCooldownGameplayEffectClass() const;

	// 技能执行层共用入口：把“谁被技能命中了”送进现有战斗尾链。
	// 这里不做命中判定，也不做是否成功释放判断，只负责把命中结果往后传。
	void ApplySkillEffectsToTargets(
		const TArray<AActor*>& Targets,
		const FHitResult* HitResult = nullptr,
		AActor* EffectCauser = nullptr,
		FName SegmentKey = NAME_None) const;

	// 技能执行层共用调试开关：执行对象必须允许，全局 CVar 也必须打开。
	bool ShouldDrawSkillDebug() const;

	// 统一从执行对象读取技能自身配置的 MetaEffects。
	void GetSkillMetaGameplayEffects(TArray<TSubclassOf<UGameplayEffect>>& OutEffects) const;

protected:
	// 如果具体技能没有单独指定冷却 GE，就回退到这里。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO|Skill Cooldown")
	TSubclassOf<UGameplayEffect> FallbackCooldownGameplayEffectClass;

private:
	void ResetHitPolicyTracking() const;

	// GAS 的 GetCooldownTags 需要返回一个可持续有效的容器地址，所以这里做缓存。
	mutable FGameplayTagContainer CachedCooldownTags;
	mutable FAOCombatHitTracker HitTracker;
};
