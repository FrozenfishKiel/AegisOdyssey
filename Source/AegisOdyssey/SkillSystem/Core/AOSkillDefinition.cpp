// Fill out your copyright notice in the Description page of Project Settings.

#include "AegisOdyssey/SkillSystem/Core/AOSkillDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOSkillDefinition)

UAOSkillDefinition::UAOSkillDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 当前阶段没有额外构造逻辑。
	// 保留显式构造函数，是为了后续如果要补默认标签或默认展示字段时有稳定入口。
}

void UAOSkillDefinition::GetCooldownIdentityTags(FGameplayTagContainer& OutTags) const
{
	OutTags.Reset();

	// 冷却身份标签的构成按“显式优先、族/组补齐”的思路组织：
	// 1. 设计显式配置的 CooldownTags 永远先进结果；
	// 2. SkillFamilyTag 和 SkillGroupTag 作为共享规则标签自动补进来；
	// 3. 这样同族/同组共享冷却的规则，就能在技能定义层稳定表达，而不再依附槽位。
	OutTags.AppendTags(CooldownTags);

	if (SkillFamilyTag.IsValid())
	{
		OutTags.AddTag(SkillFamilyTag);
	}

	if (SkillGroupTag.IsValid())
	{
		OutTags.AddTag(SkillGroupTag);
	}
}
