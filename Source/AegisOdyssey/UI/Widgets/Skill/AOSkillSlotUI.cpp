// Fill out your copyright notice in the Description page of Project Settings.

#include "AOSkillSlotUI.h"

#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "AegisOdyssey/Character/AOPawnData.h"
#include "AegisOdyssey/Input/AOInputConfig.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AegisOdyssey/Inventory/AOInventoryItemInstance.h"
#include "AegisOdyssey/UI/AOHUD.h"
#include "AegisOdyssey/UI/AOHUDViewModelComponent.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_HUD.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "PlayerMappableKeySettings.h"
#include "UserSettings/EnhancedInputUserSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOSkillSlotUI)

void UAOSkillSlotUI::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshSkillSlotDisplay();
}

bool UAOSkillSlotUI::ResolveInventoryItemContextMenuRequest(
	UAOInventoryComponent*& OutSourceInventory,
	int32& OutSourceSlotIndex,
	UAOInventoryItemInstance*& OutItemInstance) const
{
	FAOSkillSlotViewData CurrentViewData;
	const bool bHasValidViewData = ResolveCurrentSkillSlotViewData(CurrentViewData);
	if (!bHasValidViewData || CurrentViewData.SourceItemInstance == nullptr || SourceContainer == nullptr)
	{
		return false;
	}

	OutSourceInventory = SourceContainer;
	OutSourceSlotIndex = GetObservedSlotIndex();
	OutItemInstance = CurrentViewData.SourceItemInstance;
	return true;
}

const UAOInventoryItemDefinition* UAOSkillSlotUI::ResolveHoverTooltipItemDefinition() const
{
	FAOSkillSlotViewData CurrentViewData;
	if (!ResolveCurrentSkillSlotViewData(CurrentViewData) || CurrentViewData.SourceItemInstance == nullptr)
	{
		return nullptr;
	}

	return CurrentViewData.SourceItemInstance->GetItemCDO();
}

UMVVM_HUD* UAOSkillSlotUI::GetMainHUDViewModel() const
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

UAOSkillComponent* UAOSkillSlotUI::GetOwningSkillComponent() const
{
	return OwningSkillComponent.Get();
}

bool UAOSkillSlotUI::CanAcceptDraggedSourceSlotForThisSkillSlot(
	UAOInventoryComponent* InSourceContainer,
	int32 InSourceSlotIndex) const
{
	if (!InSourceContainer)
	{
		return false;
	}

	const TArray<FAOInventoryEntry> SourceEntries = InSourceContainer->GetInventoryContainer();
	if (!SourceEntries.IsValidIndex(InSourceSlotIndex))
	{
		return false;
	}

	return CanAcceptDraggedItemForThisSkillSlot(SourceEntries[InSourceSlotIndex].Instance);
}

bool UAOSkillSlotUI::CanAcceptDraggedItemForThisSkillSlot(const UAOInventoryItemInstance* SourceItemInstance) const
{
	if (!SourceItemInstance)
	{
		return false;
	}

	if (const UAOSkillComponent* SkillComponent = GetOwningSkillComponent())
	{
		return SkillComponent->CanAcceptSourceItemForSkillSlot(SourceItemInstance, GetObservedSlotIndex());
	}

	return false;
}

bool UAOSkillSlotUI::RequestEquipDraggedSourceSlotToThisSkillSlot(
	UAOInventoryComponent* InSourceContainer,
	int32 InSourceSlotIndex)
{
	UAOInventoryComponent* DraggedInventory = InSourceContainer;
	UAOInventoryComponent* DropInventory = SourceContainer;
	if (!DraggedInventory || !DropInventory)
	{
		return false;
	}

	if (!DraggedInventory->IsValidInventorySlotIndex(InSourceSlotIndex))
	{
		return false;
	}

	const int32 DropSlotIndex = GetObservedSlotIndex();
	if (!DropInventory->IsValidInventorySlotIndex(DropSlotIndex))
	{
		return false;
	}

	if (!CanAcceptDraggedSourceSlotForThisSkillSlot(DraggedInventory, InSourceSlotIndex))
	{
		return false;
	}

	RequestExchangeBetweenInventories(DraggedInventory, InSourceSlotIndex, DropInventory, DropSlotIndex);
	return true;
}

bool UAOSkillSlotUI::RequestEquipDraggedItemToThisSkillSlot(UAOInventoryItemInstance* SourceItemInstance)
{
	return CanAcceptDraggedItemForThisSkillSlot(SourceItemInstance);
}

void UAOSkillSlotUI::SetSkillSlotData(const FAOSkillSlotViewData& InSkillSlotViewData)
{
	SkillSlotViewData = InSkillSlotViewData;
	SetObservedSlotIndex(InSkillSlotViewData.SlotIndex);
	RefreshSkillSlotDisplay();
}

void UAOSkillSlotUI::SetObservedSlotIndex(int32 InObservedSlotIndex)
{
	ObservedSlotIndex = InObservedSlotIndex;
	Index = InObservedSlotIndex;
}

void UAOSkillSlotUI::SetSourceContainer(UAOInventoryComponent* InSourceContainer)
{
	SourceContainer = InSourceContainer;
}

void UAOSkillSlotUI::SetInputDisplayText(const FText& InInputDisplayText)
{
	InputDisplayText = InInputDisplayText;
	RefreshSkillSlotDisplay();
}

void UAOSkillSlotUI::SetOwningSkillComponent(UAOSkillComponent* InSkillComponent)
{
	OwningSkillComponent = InSkillComponent;
}

bool UAOSkillSlotUI::ResolveCurrentSkillSlotViewData(FAOSkillSlotViewData& OutViewData) const
{
	OutViewData = FAOSkillSlotViewData();

	if (const UAOSkillComponent* SkillComponent = GetOwningSkillComponent())
	{
		if (SkillComponent->GetSkillSlotViewData(GetObservedSlotIndex(), OutViewData))
		{
			return true;
		}
	}

	if (SkillSlotViewData.SlotIndex == GetObservedSlotIndex())
	{
		OutViewData = SkillSlotViewData;
		return true;
	}

	return false;
}

const UInputAction* UAOSkillSlotUI::GetSkillSlotInputAction() const
{
	FAOSkillSlotViewData CurrentViewData;
	if (!ResolveCurrentSkillSlotViewData(CurrentViewData) || !CurrentViewData.InputTag.IsValid())
	{
		return nullptr;
	}

	const APlayerController* OwningPlayerController = GetOwningAOPlayerController();
	if (!OwningPlayerController)
	{
		return nullptr;
	}

	const APawn* ControlledPawn = OwningPlayerController->GetPawn();
	if (!ControlledPawn)
	{
		return nullptr;
	}

	const UAOExtPawnComponent* ExtPawnComponent = UAOExtPawnComponent::FindAOExtPawnComponent(ControlledPawn);
	if (!ExtPawnComponent)
	{
		return nullptr;
	}

	const UAOPawnData* PawnData = ExtPawnComponent->GetPawnData<UAOPawnData>();
	if (!PawnData || !PawnData->InputConfig)
	{
		return nullptr;
	}

	return PawnData->InputConfig->FindInputActionForTag(CurrentViewData.InputTag, false);
}

FName UAOSkillSlotUI::GetSkillSlotInputMappingName() const
{
	const UInputAction* InputAction = GetSkillSlotInputAction();
	if (!InputAction)
	{
		return NAME_None;
	}

	const ULocalPlayer* OwningLocalPlayer = GetOwningLocalPlayer();
	if (!OwningLocalPlayer)
	{
		return NAME_None;
	}

	if (const UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = OwningLocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		for (const FEnhancedActionKeyMapping& Mapping : EnhancedInputSubsystem->GetAllPlayerMappableActionKeyMappings())
		{
			if (Mapping.Action != InputAction)
			{
				continue;
			}

			const FName MappingName = Mapping.GetMappingName();
			if (!MappingName.IsNone())
			{
				return MappingName;
			}
		}
	}

	if (const UPlayerMappableKeySettings* PlayerMappableKeySettings = InputAction->GetPlayerMappableKeySettings())
	{
		return PlayerMappableKeySettings->GetMappingName();
	}

	return NAME_None;
}

FText UAOSkillSlotUI::GetResolvedInputDisplayText() const
{
	const ULocalPlayer* OwningLocalPlayer = GetOwningLocalPlayer();
	if (OwningLocalPlayer)
	{
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = OwningLocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (UEnhancedInputUserSettings* UserSettings = EnhancedInputSubsystem->GetUserSettings())
			{
				const FName MappingName = GetSkillSlotInputMappingName();
				if (!MappingName.IsNone())
				{
					if (const FPlayerKeyMapping* CurrentMapping = UserSettings->FindCurrentMappingForSlot(MappingName, EPlayerMappableKeySlot::First))
					{
						const FKey CurrentKey = CurrentMapping->GetCurrentKey();
						if (CurrentKey.IsValid())
						{
							return CurrentKey.GetDisplayName();
						}
					}
				}
			}

			if (const UInputAction* InputAction = GetSkillSlotInputAction())
			{
				const TArray<FKey> MappedKeys = EnhancedInputSubsystem->QueryKeysMappedToAction(InputAction);
				for (const FKey& MappedKey : MappedKeys)
				{
					if (MappedKey.IsValid())
					{
						return MappedKey.GetDisplayName();
					}
				}
			}
		}
	}

	if (!InputDisplayText.IsEmpty())
	{
		return InputDisplayText;
	}

	return FText::FromString(TEXT("None"));
}

void UAOSkillSlotUI::RefreshSkillSlotDisplay()
{
	FAOSkillSlotViewData CurrentViewData;
	if (ResolveCurrentSkillSlotViewData(CurrentViewData))
	{
		SkillSlotViewData = CurrentViewData;
	}

	const bool bHasValidSkill = SkillSlotViewData.bHasSkill && SkillSlotViewData.SkillInstance != nullptr;

	if (SkillNameText)
	{
		SkillNameText->SetText(bHasValidSkill ? SkillSlotViewData.SkillName : FText::GetEmpty());
	}

	if (SkillIcon)
	{
		SkillIcon->SetBrushFromTexture(bHasValidSkill ? SkillSlotViewData.SkillIcon : nullptr, false);
		SkillIcon->SetVisibility(bHasValidSkill ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
	}

	if (InputIndex)
	{
		InputIndex->SetText(GetResolvedInputDisplayText());
	}

	HandleSkillSlotDataUpdated(bHasValidSkill);
}

bool UAOSkillSlotUI::GetSkillSlotCooldownState(float& OutTimeRemaining, float& OutTotalDuration) const
{
	OutTimeRemaining = 0.0f;
	OutTotalDuration = 0.0f;

	if (const UMVVM_HUD* HUDViewModel = GetMainHUDViewModel())
	{
		return HUDViewModel->GetSkillSlotCooldownState(GetObservedSlotIndex(), OutTimeRemaining, OutTotalDuration);
	}

	return false;
}

float UAOSkillSlotUI::GetSkillSlotCooldownRemaining() const
{
	float TimeRemaining = 0.0f;
	float TotalDuration = 0.0f;
	GetSkillSlotCooldownState(TimeRemaining, TotalDuration);
	return TimeRemaining;
}

bool UAOSkillSlotUI::IsSkillSlotOnCooldown() const
{
	float TimeRemaining = 0.0f;
	float TotalDuration = 0.0f;
	return GetSkillSlotCooldownState(TimeRemaining, TotalDuration);
}

void UAOSkillSlotUI::HandleSkillSlotDataUpdated(bool bHasValidSkill)
{
	(void)bHasValidSkill;
}
