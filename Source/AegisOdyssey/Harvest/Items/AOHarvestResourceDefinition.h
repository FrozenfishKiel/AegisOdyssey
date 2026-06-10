// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AOHarvestResourceDefinition.generated.h"

// 采集产物的正式定义类型。
// 它仍然留在现有库存物品体系里，但给“采集资源”这一类物品一个清晰的类型边界。
UCLASS(BlueprintType, Blueprintable)
class AEGISODYSSEY_API UAOHarvestResourceDefinition : public UAOInventoryItemDefinition
{
	GENERATED_BODY()

public:
	UAOHarvestResourceDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 资源标签当前主要服务后续的筛选、分类、制造输入识别和编辑器检索。
	// 它不是本轮掉落结算是否成立的核心开关。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	FGameplayTagContainer ResourceTags;
};
