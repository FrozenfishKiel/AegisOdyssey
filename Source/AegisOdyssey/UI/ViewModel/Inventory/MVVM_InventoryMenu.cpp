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
	// 写入最新库存列表，并通知所有观察该 ViewModel 的界面刷新。
	InventoryListData.InventoryList = InventoryList;
	OnInventoryListChangedDynamic.Broadcast();
}

inline TArray<FAOInventoryEntry> UMVVM_InventoryMenu::GetInventoryList() const
{
	return InventoryListData.InventoryList;
}

void UMVVM_InventoryMenu::SetQuickBarList(const TArray<FAOInventoryEntry>& NewQuickBarList)
{
	// 写入最新快捷栏列表，并通知所有观察该 ViewModel 的界面刷新。
	QuickBarData.QuickBarList = NewQuickBarList;
	OnQuickBarListChangedDynamic.Broadcast();
}

inline TArray<FAOInventoryEntry> UMVVM_InventoryMenu::GetQuickBarList() const
{
	return QuickBarData.QuickBarList;
}

void UMVVM_InventoryMenu::SetFormalEquipmentList(const TArray<FAOInventoryEntry>& NewFormalEquipmentList)
{
	// 写入最新正式装备槽位快照，并通知所有观察正式装备栏的界面刷新。
	FormalEquipmentData.FormalEquipmentList = NewFormalEquipmentList;
	OnFormalEquipmentListChangedDynamic.Broadcast();
}

inline TArray<FAOInventoryEntry> UMVVM_InventoryMenu::GetFormalEquipmentList() const
{
	return FormalEquipmentData.FormalEquipmentList;
}





/*Call On Client.*/
void UMVVM_InventoryMenu::InventoryListDataRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	OnInventoryListChangedDynamic.Broadcast();
}

void UMVVM_InventoryMenu::InventoryListDataAdd(const TArrayView<int32> AddIndices, int32 FinalSize)
{
	OnInventoryListChangedDynamic.Broadcast();
}

void UMVVM_InventoryMenu::InventoryListDataChanged(const TArrayView<int32> ChangeIndices, int32 FinalSize)
{
	OnInventoryListChangedDynamic.Broadcast();
}



void UMVVM_InventoryMenu::QuickBarDataRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	OnQuickBarListChangedDynamic.Broadcast();
}

void UMVVM_InventoryMenu::QuickBarDataAdd(const TArrayView<int32> AddIndices, int32 FinalSize)
{
	OnQuickBarListChangedDynamic.Broadcast();
}

void UMVVM_InventoryMenu::QuickBarDataChanged(const TArrayView<int32> ChangeIndices, int32 FinalSize)
{
	OnQuickBarListChangedDynamic.Broadcast();
}

void UMVVM_InventoryMenu::FormalEquipmentDataRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	OnFormalEquipmentListChangedDynamic.Broadcast();
}

void UMVVM_InventoryMenu::FormalEquipmentDataAdd(const TArrayView<int32> AddIndices, int32 FinalSize)
{
	OnFormalEquipmentListChangedDynamic.Broadcast();
}

void UMVVM_InventoryMenu::FormalEquipmentDataChanged(const TArrayView<int32> ChangeIndices, int32 FinalSize)
{
	OnFormalEquipmentListChangedDynamic.Broadcast();
}
