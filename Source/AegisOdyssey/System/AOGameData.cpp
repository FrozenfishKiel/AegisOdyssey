#include "AOGameData.h"

#include "AOAssetManager.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AegisOdyssey/Items/AOItemCatalogTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOGameData)

UAOGameData::UAOGameData()
{
}

bool UAOGameData::CacheRuntimeData()
{
	if (CachedItemCatalogDataTable == nullptr && !ItemCatalogDataTable.IsNull())
	{
		CachedItemCatalogDataTable = ItemCatalogDataTable.LoadSynchronous();
	}

	return CachedItemCatalogDataTable != nullptr;
}

const UDataTable* UAOGameData::GetItemCatalogDataTable() const
{
	return CachedItemCatalogDataTable;
}

const FAOItemCatalogRow* UAOGameData::FindItemCatalogRowById(int32 ItemId) const
{
	if (CachedItemCatalogDataTable == nullptr || CachedItemCatalogDataTable->GetRowStruct() != FAOItemCatalogRow::StaticStruct())
	{
		return nullptr;
	}

	for (const TPair<FName, uint8*>& RowPair : CachedItemCatalogDataTable->GetRowMap())
	{
		const FAOItemCatalogRow* ItemCatalogRow = reinterpret_cast<const FAOItemCatalogRow*>(RowPair.Value);
		if (ItemCatalogRow != nullptr && ItemCatalogRow->ItemId == ItemId)
		{
			return ItemCatalogRow;
		}
	}

	return nullptr;
}

const FAOItemCatalogRow* UAOGameData::FindItemCatalogRowByDefinitionClass(TSubclassOf<UAOInventoryItemDefinition> ItemDefinitionClass) const
{
	if (CachedItemCatalogDataTable == nullptr || CachedItemCatalogDataTable->GetRowStruct() != FAOItemCatalogRow::StaticStruct() || ItemDefinitionClass == nullptr)
	{
		return nullptr;
	}

	for (const TPair<FName, uint8*>& RowPair : CachedItemCatalogDataTable->GetRowMap())
	{
		const FAOItemCatalogRow* ItemCatalogRow = reinterpret_cast<const FAOItemCatalogRow*>(RowPair.Value);
		if (ItemCatalogRow != nullptr && ItemCatalogRow->ItemDefinitionClass == ItemDefinitionClass)
		{
			return ItemCatalogRow;
		}
	}

	return nullptr;
}

inline const UAOGameData& UAOGameData::Get()
{
	// 临时兜底方案：如果 AOGameData 为空，则返回一个默认实例。
	// 这样编辑器仍可正常启动，你可以在编辑器里配置正确的资源路径。
	UAOAssetManager& AssetManager = UAOAssetManager::Get();

	if (AssetManager.AOGameData.IsNull())
	{
		static UAOGameData* DefaultGameData = NewObject<UAOGameData>();
		return *DefaultGameData;
	}

	const UAOGameData& GameData = AssetManager.GetGameData();
	const_cast<UAOGameData&>(GameData).CacheRuntimeData();
	return GameData;
}
