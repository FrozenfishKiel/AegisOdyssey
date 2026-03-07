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

	OnInventoryListChangedDynamic.Broadcast();
}

inline TArray<FAOInventoryEntry> UMVVM_InventoryMenu::GetInventoryList() const
{
	return InventoryListData.InventoryList;
}

void UMVVM_InventoryMenu::SetQuickBarList(const TArray<FAOInventoryEntry>& NewQuickBarList)
{

	OnQuickBarListChangedDynamic.Broadcast();
}

inline TArray<FAOInventoryEntry> UMVVM_InventoryMenu::GetQuickBarList() const
{
	return QuickBarData.QuickBarList;
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

