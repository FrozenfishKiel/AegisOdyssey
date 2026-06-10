// Fill out your copyright notice in the Description page of Project Settings.

#include "AOSkillBarUI.h"

#include "AOSkillSlotUI.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AegisOdyssey/SkillSystem/Components/AOSkillSlotInventoryComponent.h"
#include "AegisOdyssey/UI/AOHUD.h"
#include "AegisOdyssey/UI/AOHUDViewModelComponent.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_HUD.h"
#include "Components/PanelWidget.h"
#include "EnhancedInputSubsystems.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOSkillBarUI)

void UAOSkillBarUI::NativeConstruct()
{
	Super::NativeConstruct();

	BindInputMappingRefreshDelegate();
	BindSkillObservationDelegate();
	RefreshSkillBar();
}

void UAOSkillBarUI::NativeDestruct()
{
	UnbindInputMappingRefreshDelegate();
	UnbindSkillObservationDelegate();
	Super::NativeDestruct();
}

void UAOSkillBarUI::SetDisplayContext(const FAOInventoryDisplayContext& InDisplayContext)
{
	// Skill bar is fed by the page-level display context as well.
	// Rebinding here keeps skill observation attached to the actor currently shown on this side.
	DisplayContext = InDisplayContext;
	BindSkillObservationDelegate();
	RefreshSkillBar();
}

UMVVM_HUD* UAOSkillBarUI::GetMainHUDViewModel() const
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

UAOSkillComponent* UAOSkillBarUI::GetObservedSkillComponent() const
{
	return ResolveSkillComponentFromDisplayContext(DisplayContext);
}

UAOSkillSlotInventoryComponent* UAOSkillBarUI::GetObservedSkillSlotInventory() const
{
	return ResolveSkillSlotInventoryFromDisplayContext(DisplayContext);
}

void UAOSkillBarUI::RequestRefreshFromSkillSystem()
{
	RefreshSkillBar();
}

void UAOSkillBarUI::RefreshSkillBar()
{
	if (!SkillSlotContainer)
	{
		return;
	}

	check(SkillSlotClass);

	SkillSlotContainer->ClearChildren();

	const TArray<FAOSkillSlotViewData> SkillSlotViewDataList = GetSkillSlotViewDataList();
	UAOInventoryComponent* ResolvedSourceContainer = ResolveSkillSlotContainer();
	UAOSkillComponent* SkillComponent = GetObservedSkillComponent();

	for (int32 SlotArrayIndex = 0; SlotArrayIndex < SkillSlotViewDataList.Num(); ++SlotArrayIndex)
	{
		const FAOSkillSlotViewData& SlotViewData = SkillSlotViewDataList[SlotArrayIndex];

		UAOSkillSlotUI* SkillSlotWidget = CreateWidget<UAOSkillSlotUI>(GetOwningPlayer(), SkillSlotClass);
		if (!SkillSlotWidget)
		{
			continue;
		}

		SkillSlotWidget->SetOwningSkillComponent(SkillComponent);
		SkillSlotWidget->SetSkillSlotData(SlotViewData);
		SkillSlotWidget->SetSourceContainer(ResolvedSourceContainer);
		SkillSlotWidget->SetInputDisplayText(BuildSlotInputDisplayText(SlotArrayIndex, SkillSlotViewDataList.Num()));
		SkillSlotWidget->SetObservedSlotIndex(SlotViewData.SlotIndex);
		SkillSlotContainer->AddChild(SkillSlotWidget);
	}

	HandleSkillBarRebuilt();
}

TArray<FAOSkillSlotViewData> UAOSkillBarUI::GetSkillSlotViewDataList() const
{
	if (const UAOSkillComponent* SkillComponent = GetObservedSkillComponent())
	{
		return SkillComponent->GetSkillSlotViewDataList();
	}

	return TArray<FAOSkillSlotViewData>();
}

bool UAOSkillBarUI::GetSkillSlotViewData(int32 SlotIndex, FAOSkillSlotViewData& OutViewData) const
{
	const TArray<FAOSkillSlotViewData> ViewDataList = GetSkillSlotViewDataList();
	if (!ViewDataList.IsValidIndex(SlotIndex))
	{
		return false;
	}

	OutViewData = ViewDataList[SlotIndex];
	return true;
}

void UAOSkillBarUI::HandleSkillBarRebuilt()
{
}

FText UAOSkillBarUI::BuildSlotInputDisplayText(int32 SlotIndex, int32 TotalSlotCount) const
{
	(void)SlotIndex;
	(void)TotalSlotCount;
	return FText::FromString(TEXT("None"));
}

void UAOSkillBarUI::BindSkillObservationDelegate()
{
	UAOSkillComponent* SkillComponent = GetObservedSkillComponent();
	if (!SkillComponent)
	{
		UnbindSkillObservationDelegate();
		return;
	}

	if (BoundSkillComponent.Get() == SkillComponent && SkillObservationDataChangedHandle.IsValid())
	{
		return;
	}

	UnbindSkillObservationDelegate();

	BoundSkillComponent = SkillComponent;
	SkillObservationDataChangedHandle =
		SkillComponent->OnSkillObservationChanged.AddUObject(this, &ThisClass::HandleSkillObservationDataChanged);
}

void UAOSkillBarUI::UnbindSkillObservationDelegate()
{
	if (UAOSkillComponent* SkillComponent = BoundSkillComponent.Get())
	{
		if (SkillObservationDataChangedHandle.IsValid())
		{
			SkillComponent->OnSkillObservationChanged.Remove(SkillObservationDataChangedHandle);
		}
	}

	SkillObservationDataChangedHandle.Reset();
	BoundSkillComponent.Reset();
}

void UAOSkillBarUI::HandleSkillObservationDataChanged()
{
	RefreshSkillBar();
}

void UAOSkillBarUI::BindInputMappingRefreshDelegate()
{
	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (BoundEnhancedInputSubsystem.Get() == EnhancedInputSubsystem)
			{
				return;
			}

			UnbindInputMappingRefreshDelegate();
			BoundEnhancedInputSubsystem = EnhancedInputSubsystem;
			EnhancedInputSubsystem->ControlMappingsRebuiltDelegate.AddDynamic(this, &ThisClass::HandleControlMappingsRebuilt);
		}
	}
}

void UAOSkillBarUI::UnbindInputMappingRefreshDelegate()
{
	if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = BoundEnhancedInputSubsystem.Get())
	{
		EnhancedInputSubsystem->ControlMappingsRebuiltDelegate.RemoveDynamic(this, &ThisClass::HandleControlMappingsRebuilt);
	}

	BoundEnhancedInputSubsystem.Reset();
}

void UAOSkillBarUI::HandleControlMappingsRebuilt()
{
	RefreshSkillBar();
}

UAOInventoryComponent* UAOSkillBarUI::ResolveSkillSlotContainer() const
{
	if (SourceContainer)
	{
		return SourceContainer;
	}

	return GetObservedSkillSlotInventory();
}
