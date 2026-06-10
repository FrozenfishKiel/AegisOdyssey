// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AOSkillDefinition.generated.h"

class UAOGameplayAbility;
class UAOSkillExecutionDefinition;
class UTexture2D;

/**
 * 技能静态定义资源。
 *
 * 这一层只回答“这个技能是什么”：
 * 1. 展示信息
 * 2. 技能标签与冷却身份
 * 3. 默认 AbilityClass
 * 4. 一个执行定义对象
 *
 * 它不默认承载动画驱动、事件等待之类的具体释放流程，
 * 那些都应该由具体技能 Ability 自己决定。
 */
UCLASS(BlueprintType)
class AEGISODYSSEY_API UAOSkillDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAOSkillDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 技能主名称。用于槽位显示、悬停提示和通用展示层。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText SkillName;

	// 技能说明文本。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display", meta = (MultiLine = "true"))
	FText SkillDescription;

	// 技能图标。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	TObjectPtr<UTexture2D> SkillIcon = nullptr;

	// 技能自身标签集合。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FGameplayTagContainer SkillTags;

	// 技能在战斗系统中的统一主身份。
	// 第一阶段优先让战斗链显式读取这个字段，不再依赖 SkillTags 里的隐式约定。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FGameplayTag PrimarySkillTag;

	// 技能族标签。用于表达同系列技能共享的长期规则。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FGameplayTag SkillFamilyTag;

	// 技能组标签。用于表达同组技能共享的系统限制。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FGameplayTag SkillGroupTag;

	// 显式声明的额外冷却身份标签。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown")
	FGameplayTagContainer CooldownTags;

	// 基础冷却时长。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float CooldownDuration = 0.0f;

	// 默认技能等级。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill", meta = (ClampMin = "1"))
	int32 DefaultSkillLevel = 1;

	// 对应的 GameplayAbility 类。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TSubclassOf<UAOGameplayAbility> AbilityClass = nullptr;

	// 技能执行定义对象。
	// 这里不再用总枚举分发，而是直接挂一个可内联编辑的执行对象子类。
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Execution")
	TObjectPtr<UAOSkillExecutionDefinition> ExecutionDefinition = nullptr;

public:
	// 统一构建技能共享冷却身份标签集合。
	void GetCooldownIdentityTags(FGameplayTagContainer& OutTags) const;

	bool HasConfiguredCooldown() const { return CooldownDuration > 0.0f; }

	// SkillDefinition 只回答“有没有执行对象”，不再回答“属于哪种执行枚举”。
	UFUNCTION(BlueprintPure, Category = "Skill")
	UAOSkillExecutionDefinition* GetExecutionDefinition() const { return ExecutionDefinition; }
};
