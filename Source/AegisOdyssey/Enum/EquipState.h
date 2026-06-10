// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EquipState.generated.h"

/**
 * 装备状态枚举
 * 用于表示物品或武器的装备状态
 */
UENUM(BlueprintType)
enum class EEquipState : uint8
{
	None UMETA(DisplayName = "None"),
	Sword UMETA(DisplayName = "Sword"),
	Shield UMETA(DisplayName = "Shield"),
	Spear UMETA(DisplayName = "Spear"),
	Axe UMETA(DisplayName = "Axe"),
};
