// Fill out your copyright notice in the Description page of Project Settings.

#include "AOCraftingWidgetBase.h"

#include "AegisOdyssey/UI/ViewModel/AOCombatFeedbackBlueprintLibrary.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_Crafting.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCraftingWidgetBase)

void UAOCraftingWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	// 构造时先接上当前 ViewModel 观察，再立刻消费一次已有快照。
	BindCraftingObservationDelegate();
	RefreshWidgetFromViewModelSnapshot();
}

void UAOCraftingWidgetBase::NativeDestruct()
{
	UnbindCraftingObservationDelegate();
	Super::NativeDestruct();
}

UMVVM_Crafting* UAOCraftingWidgetBase::GetCraftingViewModel() const
{
	return UAOCombatFeedbackBlueprintLibrary::GetCraftingViewModel(this);
}

void UAOCraftingWidgetBase::RefreshFromCraftingViewModel()
{
	BindCraftingObservationDelegate();
	RefreshWidgetFromViewModelSnapshot();
}

void UAOCraftingWidgetBase::HandleCraftingViewModelChanged()
{
}

void UAOCraftingWidgetBase::BindCraftingObservationDelegate()
{
	UMVVM_Crafting* CraftingViewModel = GetCraftingViewModel();
	if (CraftingViewModel == nullptr)
	{
		UnbindCraftingObservationDelegate();
		return;
	}

	if (BoundCraftingViewModel.Get() == CraftingViewModel && CraftingObservationChangedHandle.IsValid())
	{
		return;
	}

	UnbindCraftingObservationDelegate();

	BoundCraftingViewModel = CraftingViewModel;
	CraftingObservationChangedHandle = CraftingViewModel->OnCraftingObservationChanged.AddUObject(
		this, &ThisClass::HandleCraftingObservationChanged);
}

void UAOCraftingWidgetBase::UnbindCraftingObservationDelegate()
{
	if (UMVVM_Crafting* CraftingViewModel = BoundCraftingViewModel.Get())
	{
		if (CraftingObservationChangedHandle.IsValid())
		{
			CraftingViewModel->OnCraftingObservationChanged.Remove(CraftingObservationChangedHandle);
		}
	}

	CraftingObservationChangedHandle.Reset();
	BoundCraftingViewModel.Reset();
}

void UAOCraftingWidgetBase::RefreshWidgetFromViewModelSnapshot()
{
	// 真正如何把快照落到界面上，由子类实现。
	HandleCraftingViewModelChanged();
}

void UAOCraftingWidgetBase::HandleCraftingObservationChanged()
{
	// ViewModel 广播“制造观察快照已更新”后，当前 Widget 只重新消费快照。
	RefreshWidgetFromViewModelSnapshot();
}
