// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AegisOdyssey/UI/ViewModel/AOMVVMViewModelBase.h"
#include "MVVM_InventoryMenu.generated.h"

// 背包栏位列表的表现层快照。
// 这里只承载 UI 需要观察的一组库存条目，不承担库存系统运行时真相职责。
// 这类结构的意义，是把表现层需要的数据整理成稳定快照，避免 UI 直接贴着底层库存容器跑。
USTRUCT(BlueprintType)
struct FMVVM_InventoryData
{
	GENERATED_BODY()
	FMVVM_InventoryData() {}

	// 当前背包列表在 UI 侧看到的槽位快照。
	// 顺序、内容和数量都服务于当前界面展示，不代表底层一定暴露相同组织方式。
	UPROPERTY(BlueprintReadWrite)
	TArray<FAOInventoryEntry> InventoryList;
};

// 快捷栏条目列表的表现层快照。
// 用法与背包快照一致，只是服务对象变成快捷栏区域。
USTRUCT(BlueprintType)
struct FMVVM_QuickBarData
{
	GENERATED_BODY()
	FMVVM_QuickBarData() {}

	// 当前快捷栏在 UI 侧看到的槽位快照。
	// UI 层应把它当作当前显示快照，而不是拿它反推底层库存结构。
	UPROPERTY(BlueprintReadWrite)
	TArray<FAOInventoryEntry> QuickBarList;
};

// 正式装备栏条目列表的表现层快照。
// 这份数据专门服务正式装备栏 UI，不直接承担装备系统运行时状态源的职责。
USTRUCT(BlueprintType)
struct FMVVM_FormalEquipmentData
{
	GENERATED_BODY()
	FMVVM_FormalEquipmentData() {}

	// 当前正式装备栏在 UI 侧看到的槽位快照。
	// 它是给界面展示和绑定消费的结构化结果，而不是装备系统的底层数据结构本体。
	UPROPERTY(BlueprintReadWrite)
	TArray<FAOInventoryEntry> FormalEquipmentList;
};

// 库存总菜单的聚合 ViewModel。
// 它同时承接背包、快捷栏、正式装备栏三类列表快照，并为右键菜单提供独立子 ViewModel 入口。
// 可以把它理解成库存主界面的“总数据门面”，负责把多个库存区域的表现层状态收在同一处。
// 这个聚合 ViewModel 现在只负责背包、快捷栏、正式装备栏这些列表快照。
// 右键菜单 ViewModel 已经从这里拆出，改为收口到 InventoryComponent 主链，避免列表 VM 和菜单 VM 再互相持有。
UCLASS()
	class AEGISODYSSEY_API UMVVM_InventoryMenu : public UAOMVVMViewModelBase
{
	GENERATED_BODY()
public:
	// 快捷栏列表变动时对外广播。
	// 这是给非 FieldNotify 订阅方使用的列表级刷新通知。
	DECLARE_MULTICAST_DELEGATE(FOnQuickBarListChangedDynamic);

	// 背包列表变动时对外广播。
	// 主要给需要监听整组列表变化的界面层或桥接层使用。
	DECLARE_MULTICAST_DELEGATE(FOnInventoryListChangedDynamic);

	// 正式装备栏列表变动时对外广播。
	// 这类广播表达的是“这一整块区域需要刷新”，而不是单槽位精细更新信号。
	DECLARE_MULTICAST_DELEGATE(FOnFormalEquipmentListChangedDynamic);


public:
	// 用新的背包快照覆盖当前 ViewModel，并通知观察者刷新。
	// 外部调用方应在底层库存已经完成同步后再调用这里，而不是把这里当作库存写入口。
	void SetInventoryList(const TArray<FAOInventoryEntry>& InventoryList);

	// 返回当前背包列表快照。
	// 这里返回的是表现层数据，不保证和底层容器共享引用或共享生命周期。
	UFUNCTION(BlueprintPure , FieldNotify)
	inline TArray<FAOInventoryEntry> GetInventoryList() const;
public:
	// 用新的快捷栏快照覆盖当前 ViewModel，并通知观察者刷新。
	// 这一步只更新 ViewModel 内的 UI 快照，不执行快捷栏业务逻辑。
	void SetQuickBarList(const TArray<FAOInventoryEntry>& NewQuickBarList);

	// 返回当前快捷栏列表快照。
	// 蓝图或 Widget 侧应把它当成只读显示数据使用。
	UFUNCTION(BlueprintPure , FieldNotify)
	inline TArray<FAOInventoryEntry> GetQuickBarList() const;
public:
	// 用新的正式装备栏快照覆盖当前 ViewModel，并通知观察者刷新。
	// 这份数据只服务表现层观察，不承担正式装备系统运行时真相职责。
	void SetFormalEquipmentList(const TArray<FAOInventoryEntry>& NewFormalEquipmentList);

	// 返回当前正式装备栏列表快照。
	// 供正式装备栏 UI、提示区或其他展示层直接消费。
	UFUNCTION(BlueprintPure, FieldNotify)
	inline TArray<FAOInventoryEntry> GetFormalEquipmentList() const;
public:
	// 背包栏位列表的 UI 快照缓存。
	// 这一块是背包区域在表现层的统一落点。
	FMVVM_InventoryData InventoryListData;

	// 背包列表发生删除时的回调入口。
	// 参数提供了底层变更信息，但当前实现统一退化成整组刷新，不在这里做精细差量分发。
	void InventoryListDataRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);

	// 背包列表发生新增时的回调入口。
	// 这里的职责仍然是刷新表现层快照，而不是执行新增业务本身。
	void InventoryListDataAdd(const TArrayView<int32> AddIndices, int32 FinalSize);

	// 背包列表已有条目发生变化时的回调入口。
	// 通常用于已有槽位内容更新后的 UI 快照同步。
	void InventoryListDataChanged(const TArrayView<int32> ChangeIndices, int32 FinalSize);
public:
	// 快捷栏列表的 UI 快照缓存。
	// 这是快捷栏区域的表现层数据仓位。
	FMVVM_QuickBarData QuickBarData;

	// 快捷栏列表发生删除时的回调入口。
	// 当前侧重统一刷新，不在这里做复杂的局部更新优化。
	void QuickBarDataRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);

	// 快捷栏列表发生新增时的回调入口。
	// 用于把新增后的底层状态重新投影成快捷栏 UI 快照。
	void QuickBarDataAdd(const TArrayView<int32> AddIndices, int32 FinalSize);

	// 快捷栏列表已有条目发生变化时的回调入口。
	// 常见场景包括快捷栏槽位被替换、堆叠变化或状态刷新。
	void QuickBarDataChanged(const TArrayView<int32> ChangeIndices, int32 FinalSize);
public:
	// 正式装备栏列表的 UI 快照缓存。
	// 这是正式装备区域在表现层的集中快照。
	FMVVM_FormalEquipmentData FormalEquipmentData;

	// 正式装备栏列表发生删除时的回调入口。
	// 这里接到底层变更通知后，会转成正式装备区域的 UI 刷新。
	void FormalEquipmentDataRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);

	// 正式装备栏列表发生新增时的回调入口。
	// 只负责表现层同步，不负责正式装备的业务入槽流程。
	void FormalEquipmentDataAdd(const TArrayView<int32> AddIndices, int32 FinalSize);

	// 正式装备栏列表已有条目发生变化时的回调入口。
	// 用来承接现有装备槽内容更新后的表现层同步。
	void FormalEquipmentDataChanged(const TArrayView<int32> ChangeIndices, int32 FinalSize);

public:
	// 快捷栏列表刷新广播。
	// 适合给非 MVVM 绑定路径的界面逻辑做整块刷新监听。
	FOnQuickBarListChangedDynamic OnQuickBarListChangedDynamic;

	// 背包列表刷新广播。
	// 当背包区域需要整组重绘或重绑时，可直接订阅它。
	FOnInventoryListChangedDynamic OnInventoryListChangedDynamic;

	// 正式装备栏列表刷新广播。
	// 用于通知正式装备区域相关视图或桥接层刷新。
	FOnFormalEquipmentListChangedDynamic OnFormalEquipmentListChangedDynamic;

private:
	// 网络复制注册入口。
	// 当前类本身支持联网，但它现在承载的只是列表快照语义，不负责右键菜单状态复制。
	// 这里保留网络接入能力，是为了兼容库存列表 ViewModel 现有的联网使用方式。
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// 声明该 ViewModel 允许参与网络支持体系。
	// 这里表达的是对象本身允许挂在联网环境中工作，不代表所有子状态都走网络复制。
	virtual bool IsSupportedForNetworking() const override{return true;}

};
