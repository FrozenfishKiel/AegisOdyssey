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

	UPROPERTY(BlueprintReadWrite)
	TArray<FAOInventoryEntry> InventoryList;
};
USTRUCT(BlueprintType)
struct FMVVM_QuickBarData
{
	GENERATED_BODY()
	FMVVM_QuickBarData() {}

	UPROPERTY(BlueprintReadWrite)
	TArray<FAOInventoryEntry> QuickBarList;
};
UCLASS()
	class AEGISODYSSEY_API UMVVM_InventoryMenu : public UAOMVVMViewModelBase
{
	GENERATED_BODY()
public:
	DECLARE_MULTICAST_DELEGATE(FOnQuickBarListChangedDynamic);
	DECLARE_MULTICAST_DELEGATE(FOnInventoryListChangedDynamic);


public:
	void SetInventoryList(const TArray<FAOInventoryEntry>& InventoryList);

	UFUNCTION(BlueprintPure , FieldNotify)
	inline TArray<FAOInventoryEntry> GetInventoryList() const;
public:
	void SetQuickBarList(const TArray<FAOInventoryEntry>& NewQuickBarList);
	UFUNCTION(BlueprintPure , FieldNotify)
	inline TArray<FAOInventoryEntry> GetQuickBarList() const;
public:
	FMVVM_InventoryData InventoryListData;
	void InventoryListDataRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void InventoryListDataAdd(const TArrayView<int32> AddIndices, int32 FinalSize);
	void InventoryListDataChanged(const TArrayView<int32> ChangeIndices, int32 FinalSize);
public:
	FMVVM_QuickBarData QuickBarData;
	void QuickBarDataRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void QuickBarDataAdd(const TArrayView<int32> AddIndices, int32 FinalSize);
	void QuickBarDataChanged(const TArrayView<int32> ChangeIndices, int32 FinalSize);

public:
	FOnQuickBarListChangedDynamic OnQuickBarListChangedDynamic;
	FOnInventoryListChangedDynamic OnInventoryListChangedDynamic;
private:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override{return true;}
};

