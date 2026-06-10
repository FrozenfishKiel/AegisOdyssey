// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "AOSkillInstance.generated.h"

class UAOInventoryItemInstance;
class UAOSkillComponent;
class UAOSkillDefinition;

/**
 * 技能运行时实例。
 *
 * 这一层表示“当前这个具体技能对象在运行时的身份和状态”。
 * 它和 SkillDefinition 的区别是：
 * 1. SkillDefinition 是静态配置；
 * 2. SkillInstance 是某一个具体来源驱动出来的运行时对象。
 *
 * 到第五阶段为止，这里除了身份、来源和装配状态之外，
 * 还要开始承担“实例视角下的冷却语义入口”。
 */
UCLASS(BlueprintType, Blueprintable)
class AEGISODYSSEY_API UAOSkillInstance : public UObject
{
	GENERATED_BODY()

public:
	UAOSkillInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }

	// 初始化技能实例的静态定义与来源锚点。
	// 这一入口只做“身份建档”，不做装槽、不做授予，也不主动创建冷却状态。
	void InitializeSkillInstance(UAOSkillDefinition* InSkillDefinition, UAOInventoryItemInstance* InSourceItemInstance);

	// 更新运行时装配状态。
	// 当前阶段统一把“是否已装”和“当前槽位索引”收口在这里，避免多个系统各自维护一份状态。
	void SetEquippedState(bool bInEquipped, int32 InSlotIndex);

	UFUNCTION(BlueprintPure, Category = "Skill")
	UAOSkillDefinition* GetSkillDefinition() const;

	UFUNCTION(BlueprintPure, Category = "Skill")
	TSubclassOf<UAOSkillDefinition> GetSkillDefinitionClass() const;

	UFUNCTION(BlueprintPure, Category = "Skill")
	UAOInventoryItemInstance* GetSourceItemInstance() const { return SourceItemInstance; }

	UFUNCTION(BlueprintPure, Category = "Skill")
	int32 GetCurrentSlotIndex() const { return CurrentSlotIndex; }

	UFUNCTION(BlueprintPure, Category = "Skill")
	bool IsEquipped() const { return bEquipped; }

	UFUNCTION(BlueprintPure, Category = "Skill")
	int32 GetSkillLevel() const { return SkillLevel; }

	UFUNCTION(BlueprintPure, Category = "Skill")
	int32 GetSkillQuality() const { return SkillQuality; }

	// 当前实例配置出来的基础冷却时长。
	// 这里仍然不缓存运行时剩余冷却，而是把“静态定义给出来的基础时长”统一从实例层透传出来。
	UFUNCTION(BlueprintPure, Category = "Skill|Cooldown")
	float GetConfiguredCooldownDuration() const;

	// 当前实例是否真的具备可用于共享冷却查询的身份标签。
	UFUNCTION(BlueprintPure, Category = "Skill|Cooldown")
	bool HasCooldownIdentityTags() const;

	// 读取当前实例的冷却身份标签集合。
	// 这一层的意义是：后面无论 ASC 冷却查询、UI 展示，还是技能专用 Ability 应用冷却，
	// 都直接从 SkillInstance 取“我这个实例代表什么冷却身份”，而不是再去问槽位。
	void GetCooldownIdentityTags(FGameplayTagContainer& OutTags) const;

	// 读取外层技能组件，方便从实例反查所属角色的技能运行时容器。
	UAOSkillComponent* GetOwningSkillComponent() const;

private:
	// 指向技能静态定义。
	// 它专门回答“这是什么技能”，不承担来源物品那一层职责。
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAOSkillDefinition> SkillDefinition = nullptr;

	// 指向当前实例的来源物品锚点。
	// 当前方案下，背包中的物品实例本身就是技能实例身份锚点。
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAOInventoryItemInstance> SourceItemInstance = nullptr;

	// 当前技能等级。
	// 现阶段先给出最基础的运行时字段，具体成长与继承规则后续再扩展。
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = true))
	int32 SkillLevel = 1;

	// 当前技能品质。
	// 这里先只保留承载位，后续可由来源物品、强化系统或掉落词条驱动。
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = true))
	int32 SkillQuality = 0;

	// 当前实例是否已经进入某个技能槽。
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = true))
	bool bEquipped = false;

	// 当前装配到哪个槽。
	// INDEX_NONE 表示当前未装配。
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = true))
	int32 CurrentSlotIndex = INDEX_NONE;
};
