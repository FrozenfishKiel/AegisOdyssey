#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AOItemCatalogTypes.generated.h"

class UAOInventoryItemDefinition;

// 这是全项目物品总表的最小行结构。
// 第一阶段先只把“数字 ID 如何映射到物品定义/实例类型”这条入口立住，不在这里扩成完整物品系统。
USTRUCT(BlueprintType)
struct FAOItemCatalogRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	int32 ItemId = INDEX_NONE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSubclassOf<UAOInventoryItemDefinition> ItemDefinitionClass = nullptr;

};
