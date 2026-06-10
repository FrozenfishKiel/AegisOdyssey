// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Character/AOCombatHitPolicy.h"
#include "GameplayTagContainer.h"
#include "ScalableFloat.h"
#include "UObject/Object.h"
#include "AOSkillExecutionDefinition.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct FAOSkillEffectConfig
{
	GENERATED_BODY()

	// 技能命中在战斗系统中的统一攻击身份。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FGameplayTag AttackTag;

	// 伤害类型先统一带进战斗上下文，第一阶段不强行扩成完整抗性系统。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FGameplayTagContainer DamageTypeTags;

	// 第二阶段开始，技能命中如何处理重复命中由这里统一声明。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FAOCombatHitPolicy HitPolicy;

	// 命中目标后，统一复用现有战斗尾链接入这些 MetaGameplayEffects。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TArray<TSubclassOf<UGameplayEffect>> MetaGameplayEffects;
};

USTRUCT(BlueprintType)
struct FAOSkillCueConfig
{
	GENERATED_BODY()

	// 技能执行层暴露给表现层的正式瞬时事件入口。
	// 联机下应优先通过 GameplayCue 同步，而不是让具体技能自己散写多播。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue", meta = (Categories = "GameplayCue"))
	FGameplayTag ExecuteCueTag;
};

USTRUCT(BlueprintType)
struct FAOSkillDebugConfig
{
	GENERATED_BODY()

	// 资源层先声明“是否允许调试绘制”，实际是否绘制再由执行层全局开关统一裁决。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
	bool bEnableDebugDraw = false;

	// 调试图形在世界里保留多久，便于观察技能真实逻辑范围。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DebugDrawDuration = 2.0f;

	// 主范围颜色。通常用来画技能的大逻辑区域。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
	FLinearColor PrimaryDebugColor = FLinearColor::Yellow;

	// 次范围颜色。通常用来画实际命中的小范围。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
	FLinearColor SecondaryDebugColor = FLinearColor::Red;
};

USTRUCT(BlueprintType)
struct FAOSkillOriginConfig
{
	GENERATED_BODY()

	// 如果填写了有效 SocketName，就优先从角色任意 SkeletalMeshComponent 的该 Socket 取世界变换。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Origin")
	FName SocketName = NAME_None;

	// 原点局部偏移。用于微调技能生成位置。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Origin")
	FVector LocationOffset = FVector::ZeroVector;

	// 原点局部旋转偏移。用于微调技能发射朝向。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Origin")
	FRotator RotationOffset = FRotator::ZeroRotator;
};

/**
 * 技能执行定义对象基类。
 *
 * 这里不是“技能大类型”的全局枚举，而是“执行主链的承载对象”。
 * SkillDefinition 只持有一个 ExecutionDefinition，具体如何执行由子类自己表达。
 */
UCLASS(Abstract, BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew)
class AEGISODYSSEY_API UAOSkillExecutionDefinition : public UObject
{
	GENERATED_BODY()

public:
	UAOSkillExecutionDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 命中后统一交给现有战斗尾链应用的效果配置。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Execution")
	FAOSkillEffectConfig EffectConfig;

	// 技能执行层对表现层的正式配置入口。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Execution")
	FAOSkillCueConfig CueConfig;

	// 逻辑调试绘制配置。只服务于技能执行验证，不参与正式结算。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Execution")
	FAOSkillDebugConfig DebugConfig;
};
