// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Items/AOEquipmentItem.h"
#include "AOWeapon.generated.h"

// 场景中的武器拾取物 Actor。
// 它属于面向场景的世界物品层，与库存运行时的 WeaponDefinition / WeaponInstance 不是同一层语义。
UCLASS(Blueprintable, BlueprintType)
class AEGISODYSSEY_API AAOWeapon : public AAOEquipmentItem
{
	GENERATED_BODY()
public:
	virtual void InitializeActorSpawnConfig() override;
};
