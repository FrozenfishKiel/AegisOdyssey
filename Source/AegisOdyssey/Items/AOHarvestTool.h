// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Items/AOItem.h"
#include "AOHarvestTool.generated.h"

// 场景中的采集工具拾取物 Actor。
// 它和 AAOWeapon 一样，都属于面向场景的世界物品层，用来承载斧头、镐子等具体工具。
// 真正进入库存后的运行时实例类型，仍然由 ItemDefinition -> PreferredInstanceType 决定。
UCLASS(Blueprintable, BlueprintType)
class AEGISODYSSEY_API AAOHarvestTool : public AAOItem
{
	GENERATED_BODY()

public:
	virtual void InitializeActorSpawnConfig() override;
};
