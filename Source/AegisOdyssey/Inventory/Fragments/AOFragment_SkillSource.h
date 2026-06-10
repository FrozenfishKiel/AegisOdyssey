// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AOFragment_SkillSource.generated.h"

class UAOSkillDefinition;

/**
 * 技能来源碎片。
 *
 * 这是库存系统与技能系统之间的正式桥接边界：
 * 1. 物品本体仍然是普通库存物品；
 * 2. 只有带了这个碎片，技能系统才会把它识别为“技能载体”。
 *
 * 这样可以保证库存系统仍然只负责库存语义，
 * 而技能语义只是在物品定义上额外挂了一层“它可以提供哪个技能”的说明。
 */
UCLASS()
class AEGISODYSSEY_API UAOFragment_SkillSource : public UAOInventoryItemFragment
{
	GENERATED_BODY()

public:
	// 当前物品定义提供的是哪一个技能定义资产。
	// 这里的职责非常单纯：
	// 1. 物品定义仍然是物品定义；
	// 2. SkillSource 碎片只是在物品定义上额外挂一层“我会提供哪个技能”的声明；
	// 3. 它不负责运行时技能状态，也不负责替代 SkillDefinition 本身。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UAOSkillDefinition> SkillDefinition = nullptr;
};
