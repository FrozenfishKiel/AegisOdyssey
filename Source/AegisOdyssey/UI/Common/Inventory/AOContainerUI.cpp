// Fill out your copyright notice in the Description page of Project Settings.

#include "AOContainerUI.h"

#include "AOContainerSlot.h"
#include "AegisOdyssey/Interaction/AOInteractionSessionComponent.h"
#include "AegisOdyssey/Interaction/Session/AOContainerInteractionSessionModel.h"
#include "AegisOdyssey/Interaction/Session/AOInteractionSessionModel.h"
#include "AegisOdyssey/Player/AOPlayerController.h"
#include "Components/WrapBox.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOContainerUI)

void UAOContainerUI::NativeConstruct()
{
	Super::NativeConstruct();

	BindInteractionSessionComponent();
	HandleOwningSessionChanged(BoundSessionComponent ? BoundSessionComponent->GetCurrentSessionModel() : nullptr);
}

void UAOContainerUI::NativeDestruct()
{
	if (BoundSessionComponent && SessionChangedDelegateHandle.IsValid())
	{
		BoundSessionComponent->GetOnCurrentSessionChanged().Remove(SessionChangedDelegateHandle);
		SessionChangedDelegateHandle.Reset();
	}

	RebindContainerSessionModel(nullptr);
	BoundSessionComponent = nullptr;

	Super::NativeDestruct();
}

UMVVM_InventoryMenu* UAOContainerUI::GetInventoryViewModel() const
{
	return BoundContainerSessionModel ? BoundContainerSessionModel->GetContainerViewModel() : nullptr;
}

UAOContainerInteractionSessionModel* UAOContainerUI::GetContainerSessionModel() const
{
	return BoundContainerSessionModel;
}

bool UAOContainerUI::HasContainerSession() const
{
	return BoundContainerSessionModel != nullptr;
}

void UAOContainerUI::BindInteractionSessionComponent()
{
	if (BoundSessionComponent && SessionChangedDelegateHandle.IsValid())
	{
		BoundSessionComponent->GetOnCurrentSessionChanged().Remove(SessionChangedDelegateHandle);
		SessionChangedDelegateHandle.Reset();
	}

	BoundSessionComponent = nullptr;

	if (const AAOPlayerController* AOPlayerController = Cast<AAOPlayerController>(GetOwningAOPlayerController()))
	{
		BoundSessionComponent = AOPlayerController->GetInteractionSessionComponent();
	}

	if (!BoundSessionComponent)
	{
		return;
	}

	SessionChangedDelegateHandle = BoundSessionComponent->GetOnCurrentSessionChanged().AddUObject(
		this, &ThisClass::HandleOwningSessionChanged);
}

void UAOContainerUI::RebindContainerSessionModel(UAOContainerInteractionSessionModel* NewContainerSessionModel)
{
	if (BoundContainerSessionModel && ContainerDataChangedDelegateHandle.IsValid())
	{
		BoundContainerSessionModel->GetOnContainerDataChanged().Remove(ContainerDataChangedDelegateHandle);
		ContainerDataChangedDelegateHandle.Reset();
	}

	BoundContainerSessionModel = NewContainerSessionModel;

	if (!BoundContainerSessionModel)
	{
		return;
	}

	ContainerDataChangedDelegateHandle = BoundContainerSessionModel->GetOnContainerDataChanged().AddUObject(
		this, &ThisClass::HandleBoundContainerDataChanged);
}

void UAOContainerUI::HandleOwningSessionChanged(UAOInteractionSessionModel* NewSessionModel)
{
	RebindContainerSessionModel(Cast<UAOContainerInteractionSessionModel>(NewSessionModel));
	RefreshContainerSlots();
}

void UAOContainerUI::HandleBoundContainerDataChanged()
{
	RefreshContainerSlots();
}

void UAOContainerUI::RefreshContainerSlots()
{
	if (!ensureMsgf(ContainerSlotBox, TEXT("ContainerSlotBox is not bound on %s"), *GetName()))
	{
		return;
	}

	if (!ensureMsgf(ContainerSlotClass, TEXT("ContainerSlotClass is not configured on %s"), *GetName()))
	{
		return;
	}

	ContainerSlotBox->ClearChildren();

	if (!BoundContainerSessionModel)
	{
		return;
	}

	const TArray<FAOObservedInventorySlot>& ObservedSlots = BoundContainerSessionModel->GetObservedContainerSlots();
	for (const FAOObservedInventorySlot& ObservedSlot : ObservedSlots)
	{
		UAOContainerSlot* ContainerSlot = CreateWidget<UAOContainerSlot>(GetOwningPlayer(), ContainerSlotClass);
		if (!ContainerSlot)
		{
			continue;
		}

		ContainerSlot->ObservedSlot = ObservedSlot;
		ContainerSlot->SetSlotContext(
			ObservedSlot.SlotIndex,
			BoundContainerSessionModel->GetCurrentContainerInventory(),
			ObservedSlot.Instance);
		ContainerSlot->InitializeSlot();
		ContainerSlotBox->AddChild(ContainerSlot);
	}
}
