// Fill out your copyright notice in the Description page of Project Settings.

#include "AOMainUI.h"

#include "AegisOdyssey/UI/AOHUD.h"
#include "AegisOdyssey/UI/AOHUDViewModelComponent.h"
#include "AegisOdyssey/UI/ViewModel/AOCombatFeedbackBlueprintLibrary.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_CombatFeedbackFeed.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_CombatResources.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_Crafting.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_LocalCombatState.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_TargetHealthBarCollection.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOMainUI)

void UAOMainUI::NativeConstruct()
{
	Super::NativeConstruct();
	BindInventoryAcquisitionNotifications();
}

void UAOMainUI::NativeDestruct()
{
	UnbindInventoryAcquisitionNotifications();
	Super::NativeDestruct();
}

UMVVM_HUD* UAOMainUI::GetMainHUDViewModel() const
{
	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (APlayerController* SourcePC = LocalPlayer->GetPlayerController(GetWorld()))
		{
			if (UAOHUDViewModelComponent* HUDViewModelComponent = AAOHUD::FindHUDOwnedComponent<UAOHUDViewModelComponent>(SourcePC))
			{
				return HUDViewModelComponent->GetHUDMVVM();
			}
		}
	}

	return nullptr;
}

UMVVM_CombatResources* UAOMainUI::GetCombatResourcesViewModel() const
{
	if (const UMVVM_HUD* HUDViewModel = GetMainHUDViewModel())
	{
		return HUDViewModel->GetCombatResourcesViewModel();
	}

	return nullptr;
}

UMVVM_LocalCombatState* UAOMainUI::GetLocalCombatStateViewModel() const
{
	if (const UMVVM_HUD* HUDViewModel = GetMainHUDViewModel())
	{
		return HUDViewModel->GetLocalCombatStateViewModel();
	}

	return nullptr;
}

UMVVM_CombatFeedbackFeed* UAOMainUI::GetCombatFeedbackFeedViewModel() const
{
	if (const UMVVM_HUD* HUDViewModel = GetMainHUDViewModel())
	{
		return HUDViewModel->GetCombatFeedbackFeedViewModel();
	}

	return nullptr;
}

UMVVM_TargetHealthBarCollection* UAOMainUI::GetTargetHealthBarCollectionViewModel() const
{
	if (const UMVVM_HUD* HUDViewModel = GetMainHUDViewModel())
	{
		return HUDViewModel->GetTargetHealthBarCollectionViewModel();
	}

	return nullptr;
}

UMVVM_Crafting* UAOMainUI::GetCraftingViewModel() const
{
	if (const UMVVM_HUD* HUDViewModel = GetMainHUDViewModel())
	{
		return HUDViewModel->GetCraftingViewModel();
	}

	return nullptr;
}

TArray<FAOCombatFeedbackViewData> UAOMainUI::ConsumePendingCombatFeedback() const
{
	if (UMVVM_CombatFeedbackFeed* CombatFeedbackFeedViewModel = GetCombatFeedbackFeedViewModel())
	{
		return CombatFeedbackFeedViewModel->ConsumePendingCombatFeedbackList();
	}

	return UAOCombatFeedbackBlueprintLibrary::ConsumePendingCombatFeedbackFromFeed(
		UAOCombatFeedbackBlueprintLibrary::GetCombatFeedbackFeedViewModel(this));
}

TArray<FAOInventoryAcquisitionNotification> UAOMainUI::ConsumePendingInventoryAcquisition() const
{
	if (UMVVM_HUD* HUDViewModel = GetMainHUDViewModel())
	{
		return HUDViewModel->ConsumePendingInventoryAcquisitionList();
	}

	return TArray<FAOInventoryAcquisitionNotification>();
}

void UAOMainUI::HandleInventoryAcquisitionReceived(const FAOInventoryAcquisitionNotification& Notification)
{
	BP_HandleInventoryAcquisitionReceived(Notification);
}

void UAOMainUI::BindInventoryAcquisitionNotifications()
{
	UMVVM_HUD* HUDViewModel = GetMainHUDViewModel();
	if (BoundHUDViewModel == HUDViewModel)
	{
		return;
	}

	UnbindInventoryAcquisitionNotifications();

	if (HUDViewModel == nullptr)
	{
		return;
	}

	BoundHUDViewModel = HUDViewModel;
	BoundHUDViewModel->OnInventoryAcquisitionReceived.AddDynamic(this, &ThisClass::HandleInventoryAcquisitionReceived);

	// Widget 创建前可能已经收到过背包入账通知，这里把积压队列补发给表现层。
	const TArray<FAOInventoryAcquisitionNotification> PendingNotifications = BoundHUDViewModel->ConsumePendingInventoryAcquisitionList();
	for (const FAOInventoryAcquisitionNotification& Notification : PendingNotifications)
	{
		HandleInventoryAcquisitionReceived(Notification);
	}
}

void UAOMainUI::UnbindInventoryAcquisitionNotifications()
{
	if (BoundHUDViewModel != nullptr)
	{
		BoundHUDViewModel->OnInventoryAcquisitionReceived.RemoveDynamic(this, &ThisClass::HandleInventoryAcquisitionReceived);
		BoundHUDViewModel = nullptr;
	}
}
