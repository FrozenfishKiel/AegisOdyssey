// Fill out your copyright notice in the Description page of Project Settings.

#include "AegisOdyssey/SkillSystem/Core/AOSkillInstance.h"

#include "AegisOdyssey/SkillSystem/Components/AOSkillComponent.h"
#include "AegisOdyssey/SkillSystem/Core/AOSkillDefinition.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOSkillInstance)

UAOSkillInstance::UAOSkillInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAOSkillInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	// 技能实例是可复制子对象，因此需要把运行时真正要跨网络保持一致的身份字段都显式列出来。
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, SkillDefinition);
	DOREPLIFETIME(ThisClass, SourceItemInstance);
	DOREPLIFETIME(ThisClass, SkillLevel);
	DOREPLIFETIME(ThisClass, SkillQuality);
	DOREPLIFETIME(ThisClass, bEquipped);
	DOREPLIFETIME(ThisClass, CurrentSlotIndex);
}

void UAOSkillInstance::InitializeSkillInstance(UAOSkillDefinition* InSkillDefinition, UAOInventoryItemInstance* InSourceItemInstance)
{
	// 这里只做最小初始化：
	// 1. 固定技能静态定义；
	// 2. 固定来源锚点；
	// 3. 用定义里的默认值填充初始等级；
	// 4. 把实例状态重置为“未装配”。
	SkillDefinition = InSkillDefinition;
	SourceItemInstance = InSourceItemInstance;
	SkillLevel = InSkillDefinition ? FMath::Max(1, InSkillDefinition->DefaultSkillLevel) : 1;
	SkillQuality = 0;
	bEquipped = false;
	CurrentSlotIndex = INDEX_NONE;
}

UAOSkillDefinition* UAOSkillInstance::GetSkillDefinition() const
{
	return SkillDefinition;
}

TSubclassOf<UAOSkillDefinition> UAOSkillInstance::GetSkillDefinitionClass() const
{
	return SkillDefinition ? SkillDefinition->GetClass() : nullptr;
}

void UAOSkillInstance::SetEquippedState(bool bInEquipped, int32 InSlotIndex)
{
	// 统一在这里同步“已装配状态”和“槽位索引”，避免出现半状态。
	bEquipped = bInEquipped;
	CurrentSlotIndex = bInEquipped ? InSlotIndex : INDEX_NONE;
}

float UAOSkillInstance::GetConfiguredCooldownDuration() const
{
	// 实例层只负责把当前技能定义声明出来的基础冷却透传出来，
	// 不额外缓存一份冗余静态数据。
	return SkillDefinition ? FMath::Max(0.0f, SkillDefinition->CooldownDuration) : 0.0f;
}

bool UAOSkillInstance::HasCooldownIdentityTags() const
{
	FGameplayTagContainer CooldownTags;
	GetCooldownIdentityTags(CooldownTags);
	return !CooldownTags.IsEmpty();
}

void UAOSkillInstance::GetCooldownIdentityTags(FGameplayTagContainer& OutTags) const
{
	OutTags.Reset();

	// 冷却身份的真正来源仍然是 SkillDefinition。
	// SkillInstance 只是把“这份定义在当前实例上的冷却语义”稳定向外暴露出来，供 ASC、UI 和 Ability 统一查询。
	if (SkillDefinition)
	{
		SkillDefinition->GetCooldownIdentityTags(OutTags);
	}
}

UAOSkillComponent* UAOSkillInstance::GetOwningSkillComponent() const
{
	// 技能实例的 Outer 就是技能组件本身，因此这里可以稳定反查回拥有它的运行时容器。
	return Cast<UAOSkillComponent>(GetOuter());
}
