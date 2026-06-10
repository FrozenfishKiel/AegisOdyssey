// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Equipment/AOEquipmentInstance.h"
#include "AOHarvestToolInstance.generated.h"

class UAOHarvestToolDefinition;
class UAOHarvestToolFragment;
class UAOHarvestToolProfile;

// 采集工具的正式运行时实例。
// 它沿用现有装备实例体系，但给“这把具体工具当前是什么状态”留出独立实例层入口。
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced)
class AEGISODYSSEY_API UAOHarvestToolInstance : public UAOEquipmentInstance
{
	GENERATED_BODY()

public:
	UAOHarvestToolInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 返回这把具体工具实例当前绑定的采集工具定义。
	UFUNCTION(BlueprintPure, Category = "AO|Harvest")
	const UAOHarvestToolDefinition* GetHarvestToolDefinition() const;

	// 返回工具定义里的采集配置块。
	// 当前主链优先通过实例暴露它，后续实例层扩展时不需要再回头改入口协议。
	UFUNCTION(BlueprintPure, Category = "AO|Harvest")
	const UAOHarvestToolFragment* GetHarvestToolFragment() const;

	// 返回这把具体工具在采集规则里的机械语义身份。
	UFUNCTION(BlueprintPure, Category = "AO|Harvest")
	const UAOHarvestToolProfile* GetHarvestToolProfile() const;
};
