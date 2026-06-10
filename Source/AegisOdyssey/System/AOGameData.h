#pragma once

#include "Engine/DataAsset.h"
#include "AOGameData.generated.h"

class UGameplayEffect;
class UDataTable;
class UAOInventoryItemDefinition;
struct FAOItemCatalogRow;

// AegisOdyssey 的全局游戏数据资源。
// 当前这里只保留物品总表入口，制造配方已经收回角色 PawnData。
UCLASS(BlueprintType, Const, Meta = (DisplayName = "AO GameData", ShortTooltip = "Data Asset Containing Global Game Data."))
class AEGISODYSSEY_API UAOGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAOGameData();

	static const UAOGameData& Get();
	bool CacheRuntimeData();
	const UDataTable* GetItemCatalogDataTable() const;
	const FAOItemCatalogRow* FindItemCatalogRowById(int32 ItemId) const;
	const FAOItemCatalogRow* FindItemCatalogRowByDefinitionClass(TSubclassOf<UAOInventoryItemDefinition> ItemDefinitionClass) const;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("AOGameData"), GetFName());
	}

public:
	UPROPERTY(EditDefaultsOnly, Category = "AO GameData", meta = (DisplayName = "Damage Gameplay Effect (Set By Caller)"))
	TSoftClassPtr<UGameplayEffect> DamageGameplayEffect_SetByCaller;

	UPROPERTY(EditDefaultsOnly, Category = "AO GameData", meta = (EditCondition = "HealGameplayEffect (Set By Caller)"))
	TSoftClassPtr<UGameplayEffect> HealGameplayEffect_SetByCaller;

	UPROPERTY(EditDefaultsOnly, Category = "AO GameData", meta = (EditCondition = "DynamicTagGameplayEffect (Set By Caller)"))
	TSoftClassPtr<UGameplayEffect> DynamicTagGameplayEffect_SetByCaller;

	// 全局物品总表入口，供 ItemId -> ItemDefinition 查询。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AO GameData|Crafting")
	TSoftObjectPtr<UDataTable> ItemCatalogDataTable;

private:
	// 运行时缓存后的物品总表。
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> CachedItemCatalogDataTable = nullptr;
};
