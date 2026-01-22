// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AegisOdyssey/UI/ViewModel/AOMVVMViewModelBase.h"
#include "MVVM_InventoryMenu.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FMVVM_InventoryData
{
	GENERATED_BODY()
	FMVVM_InventoryData() {}

	bool operator==(const FMVVM_InventoryData& other) const
	{
		if (InventoryList.Num() != other.InventoryList.Num()) return false;

		for (int32 i = 0 ; i < InventoryList.Num() ; i++)
		{
			const FAOInventoryEntry& OtherInventoryList = InventoryList[i];
			if (InventoryList[i] != other.InventoryList[i]) return false;
		}
		return true;
	}
	
	UPROPERTY(BlueprintReadWrite)
	TArray<FAOInventoryEntry> InventoryList;
};
USTRUCT(BlueprintType)
struct FMVVM_QuickBarData
{
	GENERATED_BODY()
	FMVVM_QuickBarData() {}

	bool operator==(const FMVVM_QuickBarData& other) const
	{
		if (QuickBarList.Num() != other.QuickBarList.Num()) return false;

		for (int32 i = 0 ; i < QuickBarList.Num() ; i++)
		{
			const FAOInventoryEntry& OtherInventoryList = QuickBarList[i];
			if (QuickBarList[i] != other.QuickBarList[i]) return false;
		}
		return true;
	}
	
	UPROPERTY(BlueprintReadWrite)
	TArray<FAOInventoryEntry> QuickBarList;
};
UCLASS()
	class AEGISODYSSEY_API UMVVM_InventoryMenu : public UAOMVVMViewModelBase
{
	GENERATED_BODY()
public:
	void SetInventoryList(const TArray<FAOInventoryEntry>& InventoryList);
	void SetInventoryListData(const FMVVM_InventoryData& NewInventoryListData);

	UFUNCTION(BlueprintPure , FieldNotify)
	inline TArray<FAOInventoryEntry> GetInventoryList() const;
public:
	void SetQuickBarData(const FMVVM_QuickBarData& NewQuickBarData);
	void SetQuickBarList(const TArray<FAOInventoryEntry>& NewQuickBarList);
	UFUNCTION(BlueprintPure , FieldNotify)
	inline TArray<FAOInventoryEntry> GetQuickBarList() const;
public:
	UPROPERTY(BlueprintReadOnly , FieldNotify , Setter  , meta = (AllowPrivateAccess))
	FMVVM_InventoryData InventoryListData;
	void InventoryListDataRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void InventoryListDataAdd(const TArrayView<int32> AddIndices, int32 FinalSize);
	void InventoryListDataChanged(const TArrayView<int32> ChangeIndices, int32 FinalSize,const TArray<FAOInventoryEntry>& ChangeEntryList);
public:
	UPROPERTY(BlueprintReadOnly , FieldNotify , Setter   , meta = (AllowPrivateAccess))
	FMVVM_QuickBarData QuickBarData;
	void QuickBarDataRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void QuickBarDataAdd(const TArrayView<int32> AddIndices, int32 FinalSize);
	void QuickBarDataChanged(const TArrayView<int32> ChangeIndices, int32 FinalSize,const TArray<FAOInventoryEntry>& ChangeEntryList);
private:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override{return true;}
};

