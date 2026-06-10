// Fill out your copyright notice in the Description page of Project Settings.

#include "AOContainerInteractionSessionModel.h"

#include "AegisOdyssey/Equipment/AOQuickBarComponent.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentManagerComponent.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentSlotInventoryComponent.h"
#include "AegisOdyssey/Interaction/AOInteractionSessionComponent.h"
#include "AegisOdyssey/Inventory/AOBackPackComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryItemInstance.h"
#include "AegisOdyssey/Inventory/InventoryInterface.h"
#include "AegisOdyssey/SkillSystem/Components/AOSkillComponent.h"
#include "AegisOdyssey/SkillSystem/Components/AOSkillSlotInventoryComponent.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryMenu.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOContainerInteractionSessionModel)

void UAOContainerInteractionSessionModel::InitializeContainerSession(
	AActor* InInteractableActor,
	UAOInventoryComponent* InInventoryComponent)
{
	InteractableActor = InInteractableActor;
	ObservedInventoryComponent = InInventoryComponent;
	RegisterObservedTargetComponents(InInteractableActor);

	if (!ObservedInventoryComponent.IsValid())
	{
		ResolveInventoryComponentFromInteractableActor();
	}
}

void UAOContainerInteractionSessionModel::ApplyObservedSlotsSnapshot(
	const TArray<FAOObservedInventorySlot>& InObservedSlots)
{
	ObservedContainerSlots = InObservedSlots;
	BroadcastObservedContainerChanged();
}

void UAOContainerInteractionSessionModel::ActivateSession(UAOInteractionSessionComponent* InOwnerSessionComponent)
{
	Super::ActivateSession(InOwnerSessionComponent);
	EnsureViewModel();
	BindToObservedInventoryChanges();
	RefreshObservedContainer();
}

void UAOContainerInteractionSessionModel::DeactivateSession()
{
	UnbindFromObservedInventoryChanges();
	ObservedInventoryComponent = nullptr;
	ObservedBackPackComponent = nullptr;
	ObservedQuickBarComponent = nullptr;
	ObservedFormalEquipmentSlotInventory = nullptr;
	ObservedFormalEquipmentManager = nullptr;
	ObservedSkillComponent = nullptr;
	ObservedSkillSlotInventory = nullptr;
	ObservedContainerSlots.Reset();
	BroadcastObservedContainerChanged();

	Super::DeactivateSession();
}

void UAOContainerInteractionSessionModel::RefreshObservedContainer()
{
	if (const UAOInteractionSessionComponent* OwnerComponent = GetOwnerSessionComponent())
	{
		if (!OwnerComponent->GetOwner() || !OwnerComponent->GetOwner()->HasAuthority())
		{
			return;
		}
	}

	if (!ObservedInventoryComponent.IsValid())
	{
		ResolveInventoryComponentFromInteractableActor();
	}

	SetObservedSlotsFromInventory(ObservedInventoryComponent.Get());
	BroadcastObservedContainerChanged();

	if (UAOInteractionSessionComponent* OwnerComponent = GetOwnerSessionComponent())
	{
		OwnerComponent->SyncCurrentSessionToReplication();
	}
}

bool UAOContainerInteractionSessionModel::HasObservedContainer() const
{
	return ObservedInventoryComponent.IsValid();
}

bool UAOContainerInteractionSessionModel::IsObservedTargetInventoryComponent(const UAOInventoryComponent* InventoryComponent) const
{
	if (InventoryComponent == nullptr)
	{
		return false;
	}

	return ObservedInventoryComponent.Get() == InventoryComponent
		|| ObservedBackPackComponent.Get() == InventoryComponent
		|| ObservedQuickBarComponent.Get() == InventoryComponent
		|| ObservedFormalEquipmentSlotInventory.Get() == InventoryComponent
		|| ObservedSkillSlotInventory.Get() == InventoryComponent;
}

bool UAOContainerInteractionSessionModel::UsesObservedTargetInventory(
	const UAOInventoryComponent* FirstInventoryComponent,
	const UAOInventoryComponent* SecondInventoryComponent) const
{
	return IsObservedTargetInventoryComponent(FirstInventoryComponent)
		|| IsObservedTargetInventoryComponent(SecondInventoryComponent);
}

UAOBackPackComponent* UAOContainerInteractionSessionModel::GetObservedBackPackComponent() const
{
	return ObservedBackPackComponent.Get();
}

UAOQuickBarComponent* UAOContainerInteractionSessionModel::GetObservedQuickBarComponent() const
{
	return ObservedQuickBarComponent.Get();
}

UMVVM_InventoryMenu* UAOContainerInteractionSessionModel::GetTargetQuickBarViewModel() const
{
	if (const UAOQuickBarComponent* QuickBarComponent = GetObservedQuickBarComponent())
	{
		return QuickBarComponent->GetQuickBarViewModel();
	}

	return nullptr;
}

UAOFormalEquipmentSlotInventoryComponent* UAOContainerInteractionSessionModel::GetObservedFormalEquipmentSlotInventory() const
{
	return ObservedFormalEquipmentSlotInventory.Get();
}

UMVVM_InventoryMenu* UAOContainerInteractionSessionModel::GetTargetFormalEquipmentViewModel() const
{
	if (const UAOFormalEquipmentSlotInventoryComponent* FormalEquipmentInventory = GetObservedFormalEquipmentSlotInventory())
	{
		return FormalEquipmentInventory->GetFormalEquipmentViewModel();
	}

	return nullptr;
}

UAOFormalEquipmentManagerComponent* UAOContainerInteractionSessionModel::GetObservedFormalEquipmentManager() const
{
	return ObservedFormalEquipmentManager.Get();
}

UAOSkillComponent* UAOContainerInteractionSessionModel::GetObservedSkillComponent() const
{
	return ObservedSkillComponent.Get();
}

UAOSkillSlotInventoryComponent* UAOContainerInteractionSessionModel::GetObservedSkillSlotInventory() const
{
	return ObservedSkillSlotInventory.Get();
}

TArray<FAOSkillSlotViewData> UAOContainerInteractionSessionModel::GetObservedSkillSlotViewDataList() const
{
	if (const UAOSkillComponent* SkillComponent = GetObservedSkillComponent())
	{
		return SkillComponent->GetSkillSlotViewDataList();
	}

	return TArray<FAOSkillSlotViewData>();
}

void UAOContainerInteractionSessionModel::PopulateTargetInventoryDisplayContext(FAOInventoryDisplayContext& OutDisplayContext) const
{
	OutDisplayContext = FAOInventoryDisplayContext();
	OutDisplayContext.OwnerActor = GetInteractableActor();
	OutDisplayContext.BackPackComponent = GetObservedBackPackComponent();
	OutDisplayContext.QuickBarComponent = GetObservedQuickBarComponent();
	OutDisplayContext.FormalEquipmentInventory = GetObservedFormalEquipmentSlotInventory();
	OutDisplayContext.FormalEquipmentManager = GetObservedFormalEquipmentManager();
	OutDisplayContext.SkillComponent = GetObservedSkillComponent();
	OutDisplayContext.SkillSlotInventory = GetObservedSkillSlotInventory();
}

void UAOContainerInteractionSessionModel::BroadcastObservedContainerChanged()
{
	EnsureViewModel();

	if (ContainerViewModel)
	{
		TArray<FAOInventoryEntry> SyntheticEntries;
		SyntheticEntries.Reserve(ObservedContainerSlots.Num());
		for (const FAOObservedInventorySlot& Slot : ObservedContainerSlots)
		{
			FAOInventoryEntry Entry(nullptr);
			Entry.Instance = Slot.Instance;
			Entry.StackCount = Slot.StackCount;
			SyntheticEntries.Add(Entry);
		}

		ContainerViewModel->SetInventoryList(SyntheticEntries);
		ContainerViewModel->OnInventoryListChangedDynamic.Broadcast();
	}

	OnContainerDataChanged.Broadcast();
}

void UAOContainerInteractionSessionModel::BindToObservedInventoryChanges()
{
	const UAOInteractionSessionComponent* OwnerComponent = GetOwnerSessionComponent();
	if (OwnerComponent == nullptr || OwnerComponent->GetOwner() == nullptr || !OwnerComponent->GetOwner()->HasAuthority())
	{
		return;
	}

	if (!ObservedInventoryComponent.IsValid())
	{
		ResolveInventoryComponentFromInteractableActor();
	}

	if (!ObservedInventoryComponent.IsValid() || ObservedInventoryChangedHandle.IsValid())
	{
		return;
	}

	ObservedInventoryChangedHandle =
		ObservedInventoryComponent->OnInventoryObservedChanged.AddUObject(this, &ThisClass::HandleObservedInventoryChanged);
}

void UAOContainerInteractionSessionModel::UnbindFromObservedInventoryChanges()
{
	if (!ObservedInventoryChangedHandle.IsValid())
	{
		return;
	}

	if (ObservedInventoryComponent.IsValid())
	{
		ObservedInventoryComponent->OnInventoryObservedChanged.Remove(ObservedInventoryChangedHandle);
	}

	ObservedInventoryChangedHandle.Reset();
}

void UAOContainerInteractionSessionModel::EnsureViewModel()
{
	if (!ContainerViewModel)
	{
		ContainerViewModel = NewObject<UMVVM_InventoryMenu>(this);
	}
}

void UAOContainerInteractionSessionModel::HandleObservedInventoryChanged()
{
	RefreshObservedContainer();
}

void UAOContainerInteractionSessionModel::RegisterObservedTargetComponents(AActor* InInteractableActor)
{
	ObservedBackPackComponent = nullptr;
	ObservedQuickBarComponent = nullptr;
	ObservedFormalEquipmentSlotInventory = nullptr;
	ObservedFormalEquipmentManager = nullptr;
	ObservedSkillComponent = nullptr;
	ObservedSkillSlotInventory = nullptr;

	if (InInteractableActor == nullptr)
	{
		return;
	}

	ObservedBackPackComponent = InInteractableActor->FindComponentByClass<UAOBackPackComponent>();
	ObservedQuickBarComponent = InInteractableActor->FindComponentByClass<UAOQuickBarComponent>();
	ObservedFormalEquipmentSlotInventory = InInteractableActor->FindComponentByClass<UAOFormalEquipmentSlotInventoryComponent>();
	ObservedFormalEquipmentManager = InInteractableActor->FindComponentByClass<UAOFormalEquipmentManagerComponent>();
	ObservedSkillComponent = InInteractableActor->FindComponentByClass<UAOSkillComponent>();
	ObservedSkillSlotInventory = InInteractableActor->FindComponentByClass<UAOSkillSlotInventoryComponent>();

	if (!ObservedInventoryComponent.IsValid())
	{
		ObservedInventoryComponent = ObservedBackPackComponent.Get();
	}
}

void UAOContainerInteractionSessionModel::SetObservedSlotsFromInventory(const UAOInventoryComponent* InInventoryComponent)
{
	ObservedContainerSlots.Reset();

	if (!InInventoryComponent)
	{
		return;
	}

	const TArray<FAOInventoryEntry> SourceEntries = InInventoryComponent->GetInventoryContainer();
	ObservedContainerSlots.Reserve(SourceEntries.Num());
	for (int32 SlotIndex = 0; SlotIndex < SourceEntries.Num(); ++SlotIndex)
	{
		const FAOInventoryEntry& SourceEntry = SourceEntries[SlotIndex];

		FAOObservedInventorySlot& NewSlot = ObservedContainerSlots.AddDefaulted_GetRef();
		NewSlot.SlotIndex = SlotIndex;
		NewSlot.StackCount = SourceEntry.StackCount;
		NewSlot.Instance = SourceEntry.Instance;
		if (SourceEntry.Instance)
		{
			NewSlot.ItemDefClass = SourceEntry.Instance->ItemDef;
		}
	}
}

void UAOContainerInteractionSessionModel::ResolveInventoryComponentFromInteractableActor()
{
	UnbindFromObservedInventoryChanges();

	AActor* Interactable = InteractableActor.Get();
	if (!Interactable)
	{
		ObservedInventoryComponent = nullptr;
		return;
	}

	RegisterObservedTargetComponents(Interactable);

	if (Interactable->GetClass()->ImplementsInterface(UInventoryInterface::StaticClass()))
	{
		if (IInventoryInterface* InventoryInterface = Cast<IInventoryInterface>(Interactable))
		{
			ObservedInventoryComponent = InventoryInterface->GetInventoryComponent();
			BindToObservedInventoryChanges();
			return;
		}
	}

	ObservedInventoryComponent = nullptr;
}

bool UAOContainerInteractionSessionModel::CanExecuteMutationRequest(const FAOContainerSessionMutationRequest& MutationRequest) const
{
	switch (MutationRequest.MutationType)
	{
	case EAOContainerSessionMutationType::ExchangeInventorySlots:
		return CanExecuteExchangeMutationRequest(MutationRequest);
	case EAOContainerSessionMutationType::UseInventoryItem:
		return CanExecuteUseMutationRequest(MutationRequest);
	default:
		return false;
	}
}

bool UAOContainerInteractionSessionModel::ExecuteMutationRequestOnAuthority(const FAOContainerSessionMutationRequest& MutationRequest)
{
	const UAOInteractionSessionComponent* OwnerComponent = GetOwnerSessionComponent();
	if (OwnerComponent == nullptr || OwnerComponent->GetOwner() == nullptr || !OwnerComponent->GetOwner()->HasAuthority())
	{
		return false;
	}

	if (!CanExecuteMutationRequest(MutationRequest))
	{
		return false;
	}

	switch (MutationRequest.MutationType)
	{
	case EAOContainerSessionMutationType::ExchangeInventorySlots:
		return UAOInventoryComponent::ExecuteExchangeRequestOnAuthority(
			MutationRequest.SourceInventory,
			MutationRequest.SourceSlotIndex,
			MutationRequest.TargetInventory,
			MutationRequest.TargetSlotIndex);
	case EAOContainerSessionMutationType::UseInventoryItem:
		return MutationRequest.SourceInventory != nullptr
			&& MutationRequest.SourceInventory->TryUseItemAtSlot(MutationRequest.SourceSlotIndex, MutationRequest.UserPawn);
	default:
		return false;
	}
}

bool UAOContainerInteractionSessionModel::CanExecuteExchangeMutationRequest(
	const FAOContainerSessionMutationRequest& MutationRequest) const
{
	if (MutationRequest.SourceInventory == nullptr || MutationRequest.TargetInventory == nullptr)
	{
		return false;
	}

	if (!UsesObservedTargetInventory(MutationRequest.SourceInventory, MutationRequest.TargetInventory))
	{
		return false;
	}

	return UAOInventoryComponent::CanExecuteExchangeRequest(
		MutationRequest.SourceInventory,
		MutationRequest.SourceSlotIndex,
		MutationRequest.TargetInventory,
		MutationRequest.TargetSlotIndex);
}

bool UAOContainerInteractionSessionModel::CanExecuteUseMutationRequest(
	const FAOContainerSessionMutationRequest& MutationRequest) const
{
	if (MutationRequest.SourceInventory == nullptr || MutationRequest.UserPawn == nullptr)
	{
		return false;
	}

	if (!IsObservedTargetInventoryComponent(MutationRequest.SourceInventory))
	{
		return false;
	}

	return MutationRequest.SourceInventory->CanUseItemAtSlot(
		MutationRequest.SourceSlotIndex,
		MutationRequest.UserPawn);
}
