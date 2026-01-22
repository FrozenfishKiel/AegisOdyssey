// Fill out your copyright notice in the Description page of Project Settings.


#include "MVVM_InventoryMenu.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_InventoryMenu)

void UMVVM_InventoryMenu::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true; // 启用推送模型

}
void UMVVM_InventoryMenu::SetInventoryList(const TArray<FAOInventoryEntry>& InventoryList)
{
	FMVVM_InventoryData NewInventoryData;
	NewInventoryData.InventoryList = InventoryList;
	SetInventoryListData(NewInventoryData);
}
void UMVVM_InventoryMenu::SetInventoryListData(const FMVVM_InventoryData& NewInventoryListData)
{
	// 直接赋值新数据，不要重置为空
	if (UE_MVVM_SET_PROPERTY_VALUE_INLINE(InventoryListData , NewInventoryListData))
	{
		// 标记属性为脏，触发网络复制
		// 广播MVVM字段值变更
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetInventoryList);
	}
}

inline TArray<FAOInventoryEntry> UMVVM_InventoryMenu::GetInventoryList() const
{
	return InventoryListData.InventoryList;
}

void UMVVM_InventoryMenu::SetQuickBarList(const TArray<FAOInventoryEntry>& NewQuickBarList)
{
	FMVVM_QuickBarData NewQuickBarData;
	NewQuickBarData.QuickBarList = NewQuickBarList;
	SetQuickBarData(NewQuickBarData);
}

inline TArray<FAOInventoryEntry> UMVVM_InventoryMenu::GetQuickBarList() const
{
	return QuickBarData.QuickBarList;
}

void UMVVM_InventoryMenu::SetQuickBarData(const FMVVM_QuickBarData& NewQuickBarData)
{
	if (UE_MVVM_SET_PROPERTY_VALUE_INLINE(QuickBarData , NewQuickBarData))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetQuickBarList);
	}
}


/*Call On Client.*/
void UMVVM_InventoryMenu::InventoryListDataRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 index = 0; index < RemovedIndices.Num(); index++)
	{
		InventoryListData.InventoryList.RemoveAt(index);
	}
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetInventoryList);
}

void UMVVM_InventoryMenu::InventoryListDataAdd(const TArrayView<int32> AddIndices, int32 FinalSize)
{
	for (int32 index = 0; index < AddIndices.Num(); index++)
	{
		InventoryListData.InventoryList.EmplaceAt(AddIndices[index]);
	}
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetInventoryList);
}

void UMVVM_InventoryMenu::InventoryListDataChanged(const TArrayView<int32> ChangeIndices, int32 FinalSize,const TArray<FAOInventoryEntry>& ChangeEntryList)
{
	for (int32 index = 0; index < ChangeIndices.Num(); index++)
	{
		InventoryListData.InventoryList[index] = ChangeEntryList[index];
	}
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetInventoryList);
}



void UMVVM_InventoryMenu::QuickBarDataRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 index = 0; index < RemovedIndices.Num(); index++)
	{
		QuickBarData.QuickBarList.RemoveAt(index);
	}
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetQuickBarList);
}

void UMVVM_InventoryMenu::QuickBarDataAdd(const TArrayView<int32> AddIndices, int32 FinalSize)
{
	for (int32 index = 0; index < AddIndices.Num(); index++)
	{
		QuickBarData.QuickBarList.EmplaceAt(index);
	}
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetQuickBarList);
}

void UMVVM_InventoryMenu::QuickBarDataChanged(const TArrayView<int32> ChangeIndices, int32 FinalSize,const TArray<FAOInventoryEntry>& ChangeEntryList)
{
	for (int32 index = 0; index < ChangeIndices.Num(); index++)
	{
		QuickBarData.QuickBarList[index] = ChangeEntryList[index];
	}
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetQuickBarList);
}

