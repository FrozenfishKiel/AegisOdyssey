#include "AOInventoryUI.h"

#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Equipment/AOQuickBarComponent.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentManagerComponent.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentSlotInventoryComponent.h"
#include "AegisOdyssey/Interaction/AOInteractionSessionComponent.h"
#include "AegisOdyssey/Interaction/Session/AOContainerInteractionSessionModel.h"
#include "AegisOdyssey/Inventory/AOBackPackComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AegisOdyssey/Inventory/AOInventoryItemInstance.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_InventoryIcon.h"
#include "AegisOdyssey/Player/AOPlayerController.h"
#include "AegisOdyssey/SkillSystem/Components/AOSkillComponent.h"
#include "AegisOdyssey/SkillSystem/Components/AOSkillSlotInventoryComponent.h"
#include "AegisOdyssey/UI/ViewModel/AOCombatFeedbackBlueprintLibrary.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextMenu.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryMenu.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_Crafting.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_ItemHoverTooltip.h"
#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryItemContextMenuWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "InputCoreTypes.h"
#include "UObject/SoftObjectPath.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInventoryUI)

namespace AOInventoryUIPrivate
{
	static const FSoftClassPath DefaultInventoryItemContextMenuClassPath(
		TEXT("/Game/Games/UI/InventoryMenu/Information/WBP_InventoryContext_Action.WBP_InventoryContext_Action_C"));
}

void UAOInventoryUI::NativeConstruct()
{
	Super::NativeConstruct();
}

FReply UAOInventoryUI::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		UAOInventoryComponent* SourceInventory = nullptr;
		UAOInventoryItemInstance* ItemInstance = nullptr;
		int32 SourceSlotIndex = INDEX_NONE;
		if (!ResolveInventoryItemContextMenuRequest(SourceInventory, SourceSlotIndex, ItemInstance))
		{
			return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
		}

		RequestOpenInventoryItemContextMenu(
			SourceInventory,
			SourceSlotIndex,
			ItemInstance,
			InMouseEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UAOInventoryUI::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	ShowHoverTooltip(InGeometry);
}

void UAOInventoryUI::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	HideHoverTooltip();
	Super::NativeOnMouseLeave(InMouseEvent);
}

void UAOInventoryUI::NativeDestruct()
{
	HideHoverTooltip();
	Super::NativeDestruct();
}

bool UAOInventoryUI::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	(void)InGeometry;
	(void)InDragDropEvent;

	UAOInventoryComponent* DraggedInventory = nullptr;
	UAOInventoryItemInstance* DraggedItemInstance = nullptr;
	int32 DraggedSlotIndex = INDEX_NONE;
	if (!ResolveDraggedInventorySlotFromDropOperation(
		InOperation,
		DraggedInventory,
		DraggedSlotIndex,
		DraggedItemInstance))
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	UAOInventoryComponent* DropInventory = nullptr;
	int32 DropSlotIndex = INDEX_NONE;
	if (!ResolveDropTargetInventorySlot(DropInventory, DropSlotIndex))
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	if (DraggedInventory == nullptr || DropInventory == nullptr || DraggedSlotIndex == INDEX_NONE || DropSlotIndex == INDEX_NONE)
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	RequestExchangeBetweenInventories(DraggedInventory, DraggedSlotIndex, DropInventory, DropSlotIndex);
	return true;
}

UMVVM_InventoryMenu* UAOInventoryUI::GetInventoryViewModel() const
{
	return nullptr;
}

APlayerController* UAOInventoryUI::GetOwningAOPlayerController() const
{
	if (const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		return LocalPlayer->GetPlayerController(GetWorld());
	}

	return nullptr;
}

UAOInteractionSessionComponent* UAOInventoryUI::GetOwningInteractionSessionComponent() const
{
	if (const AAOPlayerController* OwningPlayerController = Cast<AAOPlayerController>(GetOwningAOPlayerController()))
	{
		return OwningPlayerController->GetInteractionSessionComponent();
	}

	return nullptr;
}

UAOContainerInteractionSessionModel* UAOInventoryUI::GetOwningContainerSessionModel() const
{
	if (const UAOInteractionSessionComponent* InteractionSessionComponent = GetOwningInteractionSessionComponent())
	{
		return InteractionSessionComponent->GetCurrentContainerSessionModel();
	}

	return nullptr;
}

void UAOInventoryUI::RequestAcquireCurrentInteractableOwner()
{
	if (UAOInteractionSessionComponent* InteractionSessionComponent = GetOwningInteractionSessionComponent())
	{
		InteractionSessionComponent->RequestAcquireCurrentInteractableOwner();
	}
}

void UAOInventoryUI::RequestReleaseCurrentInteractableOwner()
{
	if (UAOInteractionSessionComponent* InteractionSessionComponent = GetOwningInteractionSessionComponent())
	{
		InteractionSessionComponent->RequestReleaseCurrentInteractableOwner();
	}
}

void UAOInventoryUI::RequestExchangeBetweenInventories(
	UAOInventoryComponent* DraggedInventory,
	int32 DraggedSlotIndex,
	UAOInventoryComponent* DropInventory,
	int32 DropSlotIndex)
{
	if (!DraggedInventory || !DropInventory)
	{
		return;
	}

	if (ShouldRouteInventoryExchangeThroughInteractionSession(DraggedInventory, DropInventory))
	{
		if (UAOInteractionSessionComponent* InteractionSessionComponent = GetOwningInteractionSessionComponent())
		{
			FAOContainerSessionMutationRequest MutationRequest;
			MutationRequest.MutationType = EAOContainerSessionMutationType::ExchangeInventorySlots;
			MutationRequest.SourceInventory = DraggedInventory;
			MutationRequest.SourceSlotIndex = DraggedSlotIndex;
			MutationRequest.TargetInventory = DropInventory;
			MutationRequest.TargetSlotIndex = DropSlotIndex;
			InteractionSessionComponent->ExecuteCurrentContainerMutationRequest(MutationRequest);
			return;
		}
	}

	UAOInventoryComponent::ExecuteExchangeRequest(DraggedInventory, DraggedSlotIndex, DropInventory, DropSlotIndex);
}

void UAOInventoryUI::RequestUseInventoryItem(UAOInventoryComponent* SourceInventory, int32 SourceSlotIndex)
{
	if (SourceInventory == nullptr)
	{
		return;
	}

	if (ShouldRouteInventoryUseThroughInteractionSession(SourceInventory))
	{
		if (UAOInteractionSessionComponent* InteractionSessionComponent = GetOwningInteractionSessionComponent())
		{
			if (APawn* OwningPawn = GetOwningPlayerPawn())
			{
				FAOContainerSessionMutationRequest MutationRequest;
				MutationRequest.MutationType = EAOContainerSessionMutationType::UseInventoryItem;
				MutationRequest.SourceInventory = SourceInventory;
				MutationRequest.SourceSlotIndex = SourceSlotIndex;
				MutationRequest.UserPawn = OwningPawn;
				InteractionSessionComponent->ExecuteCurrentContainerMutationRequest(MutationRequest);
			}
		}
		return;
	}

	SourceInventory->TryUseItemAtSlot(SourceSlotIndex);
}

void UAOInventoryUI::RequestOpenInventoryItemContextMenu(
	UAOInventoryComponent* SourceInventory,
	int32 SourceSlotIndex,
	UAOInventoryItemInstance* ItemInstance,
	const FVector2D& ScreenSpacePosition)
{
	if (!InventoryItemContextMenuClass)
	{
		InventoryItemContextMenuClass =
			AOInventoryUIPrivate::DefaultInventoryItemContextMenuClassPath.TryLoadClass<UAOInventoryItemContextMenuWidget>();
	}

	if (GetOwningPlayer() == nullptr)
	{
		return;
	}

	const TArray<FAOInventoryItemContextAction> MenuActions =
		BuildInventoryItemContextActions(SourceInventory, SourceSlotIndex, ItemInstance);
	if (MenuActions.IsEmpty())
	{
		return;
	}

	if (!IsValid(ActiveContextMenuWidget))
	{
		if (!InventoryItemContextMenuClass)
		{
			return;
		}

		ActiveContextMenuWidget = CreateWidget<UAOInventoryItemContextMenuWidget>(GetOwningPlayer(), InventoryItemContextMenuClass);
		if (ActiveContextMenuWidget == nullptr)
		{
			return;
		}
	}

	if (SourceInventory == nullptr)
	{
		return;
	}

	UMVVM_InventoryItemContextMenu* ContextMenuViewModel = SourceInventory->GetOrCreateContextMenuViewModel();
	if (ContextMenuViewModel == nullptr)
	{
		return;
	}

	ContextMenuViewModel->SetRequestingInventoryUI(this);
	ContextMenuViewModel->OpenForInventorySlot(
		SourceInventory,
		SourceSlotIndex,
		ItemInstance,
		ScreenSpacePosition,
		MenuActions);

	ActiveContextMenuWidget->SetContextMenuViewModel(ContextMenuViewModel);
	ActiveContextMenuWidget->InitializeForInventorySlot();

	if (!ActiveContextMenuWidget->IsInViewport())
	{
		ActiveContextMenuWidget->AddToViewport(1000);
	}

	ActiveContextMenuWidget->SetKeyboardFocus();
}

void UAOInventoryUI::RequestOpenCraftingRecipeContextMenu(
	FName RecipeRowName,
	UAOInventoryItemDefinition* ItemDefinition,
	const FText& InfoText,
	const FVector2D& ScreenSpacePosition)
{
	if (!RecipeRowName.IsValid() || GetOwningPlayer() == nullptr)
	{
		return;
	}

	if (!InventoryItemContextMenuClass)
	{
		InventoryItemContextMenuClass =
			AOInventoryUIPrivate::DefaultInventoryItemContextMenuClassPath.TryLoadClass<UAOInventoryItemContextMenuWidget>();
	}

	if (!IsValid(ActiveContextMenuWidget))
	{
		if (!InventoryItemContextMenuClass)
		{
			return;
		}

		ActiveContextMenuWidget = CreateWidget<UAOInventoryItemContextMenuWidget>(GetOwningPlayer(), InventoryItemContextMenuClass);
		if (ActiveContextMenuWidget == nullptr)
		{
			return;
		}
	}

	TArray<FAOInventoryItemContextAction> MenuActions;

	FAOInventoryItemContextAction& CraftOneAction = MenuActions.AddDefaulted_GetRef();
	CraftOneAction.ActionType = EAOInventoryItemActionType::CraftOne;
	CraftOneAction.Label = FText::FromString(TEXT("制作一个"));
	CraftOneAction.bEnabled = true;
	CraftOneAction.bCloseMenuAfterExecute = true;
	CraftOneAction.SortOrder = 0;

	FAOInventoryItemContextAction& CraftTenAction = MenuActions.AddDefaulted_GetRef();
	CraftTenAction.ActionType = EAOInventoryItemActionType::CraftTen;
	CraftTenAction.Label = FText::FromString(TEXT("制作十个"));
	CraftTenAction.bEnabled = true;
	CraftTenAction.bCloseMenuAfterExecute = true;
	CraftTenAction.SortOrder = 1;

	FAOInventoryItemContextAction& CraftAllAction = MenuActions.AddDefaulted_GetRef();
	CraftAllAction.ActionType = EAOInventoryItemActionType::CraftAll;
	CraftAllAction.Label = FText::FromString(TEXT("制作全部"));
	CraftAllAction.bEnabled = true;
	CraftAllAction.bCloseMenuAfterExecute = true;
	CraftAllAction.SortOrder = 2;

	UMVVM_InventoryItemContextMenu* ContextMenuViewModel = NewObject<UMVVM_InventoryItemContextMenu>(this);
	if (ContextMenuViewModel == nullptr)
	{
		return;
	}

	ContextMenuViewModel->SetRequestingInventoryUI(this);

	FSlateBrush IconBrush;
	bool bHasValidIcon = false;
	if (ItemDefinition != nullptr)
	{
		if (const UAOFragment_InventoryIcon* IconFragment = ItemDefinition->FindFragmentByClass<UAOFragment_InventoryIcon>())
		{
			IconBrush = IconFragment->Brush;
			bHasValidIcon = IconBrush.GetResourceObject() != nullptr;
		}
	}

	const FText DisplayName =
		(ItemDefinition != nullptr && !ItemDefinition->DisplayName.IsNone())
			? FText::FromName(ItemDefinition->DisplayName)
			: FText::FromName(RecipeRowName);

	ContextMenuViewModel->OpenForCraftingRecipe(
		RecipeRowName,
		DisplayName,
		IconBrush,
		bHasValidIcon,
		InfoText,
		ScreenSpacePosition,
		MenuActions);

	ActiveContextMenuWidget->SetContextMenuViewModel(ContextMenuViewModel);
	ActiveContextMenuWidget->InitializeForInventorySlot();

	if (!ActiveContextMenuWidget->IsInViewport())
	{
		ActiveContextMenuWidget->AddToViewport(1000);
	}

	ActiveContextMenuWidget->SetKeyboardFocus();
}

bool UAOInventoryUI::TryResolveInventorySlotContext(
	UAOInventoryComponent*& OutSourceInventory,
	int32& OutSourceSlotIndex,
	UAOInventoryItemInstance*& OutItemInstance) const
{
	return ResolveInventoryItemContextMenuRequest(OutSourceInventory, OutSourceSlotIndex, OutItemInstance);
}

UMVVM_ItemHoverTooltip* UAOInventoryUI::GetItemHoverTooltipViewModel() const
{
	return UAOCombatFeedbackBlueprintLibrary::GetItemHoverTooltipViewModel(this);
}

TArray<FAOInventoryItemContextAction> UAOInventoryUI::BuildInventoryItemContextActions(
	UAOInventoryComponent* SourceInventory,
	int32 SourceSlotIndex,
	UAOInventoryItemInstance* ItemInstance) const
{
	TArray<FAOInventoryItemContextAction> Actions;

	if (SourceInventory && ItemInstance && SourceInventory->IsValidInventorySlotIndex(SourceSlotIndex))
	{
		const TArray<FAOInventoryEntry> SourceEntries = SourceInventory->GetInventoryContainer();
		if (SourceEntries.IsValidIndex(SourceSlotIndex))
		{
			const FAOInventoryEntry& Entry = SourceEntries[SourceSlotIndex];
			if (Entry.Instance == ItemInstance
				&& Entry.StackCount > 0
				&& Entry.Instance->GetItemCDO() != nullptr
				&& Entry.Instance->CanUseFromInventory(Entry, GetOwningPlayerPawn()))
			{
				FAOInventoryItemContextAction& UseAction = Actions.AddDefaulted_GetRef();
				UseAction.ActionType = EAOInventoryItemActionType::Use;
				UseAction.Label = FText::FromString(TEXT("使用"));
				UseAction.bEnabled = true;
				UseAction.bCloseMenuAfterExecute = true;
				UseAction.SortOrder = 0;
			}
		}
	}

	if (Actions.IsEmpty())
	{
		return Actions;
	}

	FAOInventoryItemContextAction& CloseAction = Actions.AddDefaulted_GetRef();
	CloseAction.ActionType = EAOInventoryItemActionType::Close;
	CloseAction.Label = FText::FromString(TEXT("关闭"));
	CloseAction.bEnabled = true;
	CloseAction.bCloseMenuAfterExecute = true;
	CloseAction.SortOrder = 1000;

	Actions.Sort([](const FAOInventoryItemContextAction& Left, const FAOInventoryItemContextAction& Right)
	{
		return Left.SortOrder < Right.SortOrder;
	});

	return Actions;
}

bool UAOInventoryUI::ExecuteInventoryItemContextAction(
	const FAOInventoryItemContextAction& Action,
	UAOInventoryComponent* SourceInventory,
	int32 SourceSlotIndex,
	UAOInventoryItemInstance* ItemInstance,
	FName CraftingRecipeRowName)
{
	(void)ItemInstance;

	switch (Action.ActionType)
	{
	case EAOInventoryItemActionType::Use:
		RequestUseInventoryItem(SourceInventory, SourceSlotIndex);
		return true;
	case EAOInventoryItemActionType::CraftOne:
	case EAOInventoryItemActionType::CraftTen:
	case EAOInventoryItemActionType::CraftAll:
		if (!CraftingRecipeRowName.IsValid())
		{
			return false;
		}

		if (UMVVM_Crafting* CraftingViewModel = UAOCombatFeedbackBlueprintLibrary::GetCraftingViewModel(this))
		{
			EAOCraftingRequestType RequestType = EAOCraftingRequestType::Single;
			if (Action.ActionType == EAOInventoryItemActionType::CraftTen)
			{
				RequestType = EAOCraftingRequestType::Ten;
			}
			else if (Action.ActionType == EAOInventoryItemActionType::CraftAll)
			{
				RequestType = EAOCraftingRequestType::All;
			}

			return CraftingViewModel->RequestCraftRecipe(CraftingRecipeRowName, RequestType);
		}
		return false;
	case EAOInventoryItemActionType::Close:
		return true;
	default:
		return false;
	}
}

bool UAOInventoryUI::ShouldRouteInventoryUseThroughInteractionSession(UAOInventoryComponent* SourceInventory) const
{
	if (SourceInventory == nullptr)
	{
		return false;
	}

	const UAOInteractionSessionComponent* InteractionSessionComponent = GetOwningInteractionSessionComponent();
	const UAOContainerInteractionSessionModel* ContainerSessionModel =
		InteractionSessionComponent ? InteractionSessionComponent->GetCurrentContainerSessionModel() : nullptr;
	if (ContainerSessionModel == nullptr)
	{
		return false;
	}

	return ContainerSessionModel->IsObservedTargetInventoryComponent(SourceInventory);
}

bool UAOInventoryUI::ShouldRouteInventoryExchangeThroughInteractionSession(
	UAOInventoryComponent* DraggedInventory,
	UAOInventoryComponent* DropInventory) const
{
	if (DraggedInventory == nullptr || DropInventory == nullptr)
	{
		return false;
	}

	const UAOInteractionSessionComponent* InteractionSessionComponent = GetOwningInteractionSessionComponent();
	const UAOContainerInteractionSessionModel* ContainerSessionModel =
		InteractionSessionComponent ? InteractionSessionComponent->GetCurrentContainerSessionModel() : nullptr;
	if (ContainerSessionModel == nullptr)
	{
		return false;
	}

	return ContainerSessionModel->UsesObservedTargetInventory(DraggedInventory, DropInventory);
}

void UAOInventoryUI::HandleInventoryItemContextMenuClosed(UAOInventoryItemContextMenuWidget* ClosedMenuWidget)
{
	if (ActiveContextMenuWidget != ClosedMenuWidget)
	{
		return;
	}
}

bool UAOInventoryUI::ResolveInventoryItemContextMenuRequest(
	UAOInventoryComponent*& OutSourceInventory,
	int32& OutSourceSlotIndex,
	UAOInventoryItemInstance*& OutItemInstance) const
{
	OutSourceInventory = nullptr;
	OutSourceSlotIndex = INDEX_NONE;
	OutItemInstance = nullptr;
	return false;
}

bool UAOInventoryUI::ResolveDropTargetInventorySlot(
	UAOInventoryComponent*& OutTargetInventory,
	int32& OutTargetSlotIndex) const
{
	OutTargetInventory = nullptr;
	OutTargetSlotIndex = INDEX_NONE;
	return false;
}

bool UAOInventoryUI::ResolveDraggedInventorySlotFromDropOperation(
	const UDragDropOperation* InOperation,
	UAOInventoryComponent*& OutSourceInventory,
	int32& OutSourceSlotIndex,
	UAOInventoryItemInstance*& OutItemInstance) const
{
	OutSourceInventory = nullptr;
	OutSourceSlotIndex = INDEX_NONE;
	OutItemInstance = nullptr;

	if (InOperation == nullptr || InOperation->Payload == nullptr)
	{
		return false;
	}

	const UAOInventoryUI* DraggedInventoryUI = Cast<UAOInventoryUI>(InOperation->Payload);
	if (DraggedInventoryUI == nullptr)
	{
		return false;
	}

	return DraggedInventoryUI->TryResolveInventorySlotContext(
		OutSourceInventory,
		OutSourceSlotIndex,
		OutItemInstance);
}

const UAOInventoryItemDefinition* UAOInventoryUI::ResolveHoverTooltipItemDefinition() const
{
	UAOInventoryComponent* SourceInventory = nullptr;
	UAOInventoryItemInstance* ItemInstance = nullptr;
	int32 SourceSlotIndex = INDEX_NONE;
	if (!ResolveInventoryItemContextMenuRequest(SourceInventory, SourceSlotIndex, ItemInstance))
	{
		return nullptr;
	}

	return ItemInstance != nullptr ? ItemInstance->GetItemCDO() : nullptr;
}

UAOBackPackComponent* UAOInventoryUI::ResolveBackPackComponentFromDisplayContext(
	const FAOInventoryDisplayContext& InDisplayContext) const
{
	if (UAOBackPackComponent* BackPackComponent = InDisplayContext.BackPackComponent.Get())
	{
		return BackPackComponent;
	}

	if (InDisplayContext.OwnerActor != nullptr)
	{
		return nullptr;
	}

	if (const APawn* OwningPawn = GetOwningPlayerPawn())
	{
		return OwningPawn->FindComponentByClass<UAOBackPackComponent>();
	}

	return nullptr;
}

UAOQuickBarComponent* UAOInventoryUI::ResolveQuickBarComponentFromDisplayContext(
	const FAOInventoryDisplayContext& InDisplayContext) const
{
	if (UAOQuickBarComponent* QuickBarComponent = InDisplayContext.QuickBarComponent.Get())
	{
		return QuickBarComponent;
	}

	if (InDisplayContext.OwnerActor != nullptr)
	{
		return nullptr;
	}

	if (const APawn* OwningPawn = GetOwningPlayerPawn())
	{
		return OwningPawn->FindComponentByClass<UAOQuickBarComponent>();
	}

	return nullptr;
}

UAOFormalEquipmentSlotInventoryComponent* UAOInventoryUI::ResolveFormalEquipmentInventoryFromDisplayContext(
	const FAOInventoryDisplayContext& InDisplayContext) const
{
	if (UAOFormalEquipmentSlotInventoryComponent* FormalEquipmentInventory = InDisplayContext.FormalEquipmentInventory.Get())
	{
		return FormalEquipmentInventory;
	}

	if (InDisplayContext.OwnerActor != nullptr)
	{
		return nullptr;
	}

	if (const APawn* OwningPawn = GetOwningPlayerPawn())
	{
		return OwningPawn->FindComponentByClass<UAOFormalEquipmentSlotInventoryComponent>();
	}

	return nullptr;
}

UAOFormalEquipmentManagerComponent* UAOInventoryUI::ResolveFormalEquipmentManagerFromDisplayContext(
	const FAOInventoryDisplayContext& InDisplayContext) const
{
	if (UAOFormalEquipmentManagerComponent* FormalEquipmentManager = InDisplayContext.FormalEquipmentManager.Get())
	{
		return FormalEquipmentManager;
	}

	if (InDisplayContext.OwnerActor != nullptr)
	{
		return nullptr;
	}

	if (const APawn* OwningPawn = GetOwningPlayerPawn())
	{
		return OwningPawn->FindComponentByClass<UAOFormalEquipmentManagerComponent>();
	}

	return nullptr;
}

UAOSkillComponent* UAOInventoryUI::ResolveSkillComponentFromDisplayContext(
	const FAOInventoryDisplayContext& InDisplayContext) const
{
	if (UAOSkillComponent* SkillComponent = InDisplayContext.SkillComponent.Get())
	{
		return SkillComponent;
	}

	if (InDisplayContext.OwnerActor != nullptr)
	{
		return nullptr;
	}

	if (const APawn* OwningPawn = GetOwningPlayerPawn())
	{
		return OwningPawn->FindComponentByClass<UAOSkillComponent>();
	}

	return nullptr;
}

UAOSkillSlotInventoryComponent* UAOInventoryUI::ResolveSkillSlotInventoryFromDisplayContext(
	const FAOInventoryDisplayContext& InDisplayContext) const
{
	if (UAOSkillSlotInventoryComponent* SkillSlotInventory = InDisplayContext.SkillSlotInventory.Get())
	{
		return SkillSlotInventory;
	}

	if (InDisplayContext.OwnerActor != nullptr)
	{
		return nullptr;
	}

	if (const APawn* OwningPawn = GetOwningPlayerPawn())
	{
		return OwningPawn->FindComponentByClass<UAOSkillSlotInventoryComponent>();
	}

	return nullptr;
}

void UAOInventoryUI::ShowHoverTooltip(const FGeometry& InGeometry)
{
	UMVVM_ItemHoverTooltip* TooltipViewModel = GetItemHoverTooltipViewModel();
	if (TooltipViewModel == nullptr)
	{
		return;
	}

	const UAOInventoryItemDefinition* ItemDefinition = ResolveHoverTooltipItemDefinition();
	if (ItemDefinition == nullptr)
	{
		TooltipViewModel->HideTooltip(this);
		return;
	}

	const FVector2D ScreenSpacePosition = InGeometry.LocalToAbsolute(InGeometry.GetLocalSize() * 0.5f);
	TooltipViewModel->ShowTooltip(ItemDefinition, ScreenSpacePosition, this);
}

void UAOInventoryUI::HideHoverTooltip()
{
	if (UMVVM_ItemHoverTooltip* TooltipViewModel = GetItemHoverTooltipViewModel())
	{
		TooltipViewModel->HideTooltip(this);
	}
}
