// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AOContainerInventoryComponent.generated.h"

// 通用世界容器库存组件。
// 负责把世界中的可交互容器对象正式接入现有库存系统。
UCLASS(ClassGroup = ("AO"), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOContainerInventoryComponent : public UAOInventoryComponent
{
	GENERATED_BODY()

public:
	UAOContainerInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	// 在服务端初始化容器默认槽位。
	virtual void BeginPlay() override;

	// 按照配置补齐容器槽位。
	virtual void InitializeOrRefreshInventorySlots() override;
};
