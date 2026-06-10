// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "AOCraftingWidgetBase.generated.h"

class UMVVM_Crafting;

// 制造系统 UI 的公共基类。
// 统一负责拿 Crafting ViewModel、绑定观察广播，以及在广播后刷新当前 Widget。
UCLASS(Abstract, Blueprintable)
class AEGISODYSSEY_API UAOCraftingWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 通过 HUD 侧入口拿到当前上下文里的 Crafting ViewModel。
	UFUNCTION(BlueprintPure, Category = "AO|Crafting UI")
	UMVVM_Crafting* GetCraftingViewModel() const;

	// 主动消费一次当前 ViewModel 快照。
	// 这里只刷新本地 Widget，不反向要求底层重新组装数据。
	UFUNCTION(BlueprintCallable, Category = "AO|Crafting UI")
	void RefreshFromCraftingViewModel();

protected:
	// 子类在这里把 ViewModel 快照翻译成自己的本地显示字段。
	virtual void HandleCraftingViewModelChanged();

private:
	// 绑定当前 ViewModel 的观察广播。
	void BindCraftingObservationDelegate();

	// 解除当前 ViewModel 的观察广播。
	void UnbindCraftingObservationDelegate();

	// 统一消费当前 ViewModel 快照并刷新本 Widget。
	void RefreshWidgetFromViewModelSnapshot();

	// 响应制造观察数据变更。
	void HandleCraftingObservationChanged();

private:
	// 当前已经绑定的 Crafting ViewModel。
	TWeakObjectPtr<UMVVM_Crafting> BoundCraftingViewModel;

	// 对应观察广播的委托句柄。
	FDelegateHandle CraftingObservationChangedHandle;
};
