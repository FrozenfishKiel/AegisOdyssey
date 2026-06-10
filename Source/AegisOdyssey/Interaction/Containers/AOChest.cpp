// Fill out your copyright notice in the Description page of Project Settings.

#include "AOChest.h"

#include "AOContainerInventoryComponent.h"
#include "AegisOdyssey/Interaction/AOInteractionSessionComponent.h"
#include "AegisOdyssey/Interaction/InteractionStatics.h"
#include "AegisOdyssey/Interaction/Session/AOContainerInteractionSessionModel.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryStatics.h"
#include "AegisOdyssey/Player/AOPlayerController.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Controller.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOChest)

AAOChest::AAOChest()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	InteractionBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBounds"));
	SetRootComponent(InteractionBounds);

	ChestMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestMesh"));
	ChestMesh->SetupAttachment(InteractionBounds);

	ChestInventory = CreateDefaultSubobject<UAOContainerInventoryComponent>(TEXT("ChestInventory"));
}

void AAOChest::GatherInteractionOptions(FInteractionOptionBuilder& OptionBuilder)
{
	for (const FInteractionOption& InteractionOption : InteractionOptions)
	{
		OptionBuilder.AddInteractionOption(InteractionOption);
	}
}

void AAOChest::CustomizeInteractionEventData(const FGameplayTag& InteractionEventTag, FGameplayEventData& InOutEventData)
{
	IInteractableTarget::CustomizeInteractionEventData(InteractionEventTag, InOutEventData);
}

bool AAOChest::CanExecuteInteraction(const FGameplayTag& InteractionEventTag, const FGameplayEventData& EventData) const
{
	if (!HasAuthority() || !ChestInventory)
	{
		return false;
	}

	const APawn* InteractingPawn = Cast<APawn>(const_cast<AActor*>(Cast<AActor>(EventData.Instigator.Get())));
	if (!InteractingPawn)
	{
		return false;
	}

	const int32 SelectedInteractionIndex = UInteractionStatics::GetInteractionOptionIndexFromEventData(EventData);
	return FindInteractionOptionByIndex(SelectedInteractionIndex) != nullptr;
}

bool AAOChest::ExecuteInteraction(const FGameplayTag& InteractionEventTag, const FGameplayEventData& EventData)
{
	if (!HasAuthority())
	{
		return false;
	}

	APawn* InteractingPawn = Cast<APawn>(const_cast<AActor*>(Cast<AActor>(EventData.Instigator.Get())));
	if (!InteractingPawn)
	{
		return false;
	}

	AAOPlayerController* PlayerController = Cast<AAOPlayerController>(InteractingPawn->GetController());
	if (!PlayerController || !ChestInventory)
	{
		return false;
	}

	const int32 SelectedInteractionIndex = UInteractionStatics::GetInteractionOptionIndexFromEventData(EventData);
	const FInteractionOption* SelectedOption = FindInteractionOptionByIndex(SelectedInteractionIndex);
	if (!SelectedOption)
	{
		return false;
	}

	if (UAOInteractionSessionComponent* SessionComponent = PlayerController->GetInteractionSessionComponent())
	{
		UAOContainerInteractionSessionModel* SessionModel = NewObject<UAOContainerInteractionSessionModel>(SessionComponent);
		SessionModel->InitializeContainerSession(this, ChestInventory);
		SessionModel->SetSessionWidgetClass(SelectedOption->InteractionWidgetClass.LoadSynchronous());
		SessionComponent->StartSession(SessionModel);
		return true;
	}

	return false;
}

bool AAOChest::CanInventoryAccessChest(const UAOInventoryComponent* InventoryComponent) const
{
	if (InventoryComponent == nullptr)
	{
		return false;
	}

	return IsInventoryOwnedByActiveSessionParticipant(InventoryComponent);
}

bool AAOChest::IsInventoryOwnedByActiveSessionParticipant(const UAOInventoryComponent* InventoryComponent) const
{
	if (InventoryComponent == nullptr)
	{
		return false;
	}

	const APawn* OwningPawn = Cast<APawn>(InventoryComponent->GetOwner());
	if (OwningPawn == nullptr)
	{
		return false;
	}

	const AController* OwnerController = OwningPawn->GetController();
	const AAOPlayerController* PlayerController = Cast<AAOPlayerController>(OwnerController);
	if (PlayerController == nullptr)
	{
		return false;
	}

	const UAOInteractionSessionComponent* SessionComponent = PlayerController->GetInteractionSessionComponent();
	const UAOContainerInteractionSessionModel* ContainerSessionModel =
		SessionComponent != nullptr ? SessionComponent->GetCurrentContainerSessionModel() : nullptr;
	return ContainerSessionModel != nullptr && ContainerSessionModel->GetInteractableActor() == this;
}

bool AAOChest::TransferChestSlotToInventorySlot(UAOInventoryComponent* ExternalInventory, int32 ChestSlotIndex, int32 ExternalSlotIndex)
{
	if (!ChestInventory || !ExternalInventory)
	{
		return false;
	}

	if (!CanInventoryAccessChest(ExternalInventory))
	{
		return false;
	}

	return UAOInventoryComponent::ExecuteExchangeRequest(ChestInventory, ChestSlotIndex, ExternalInventory, ExternalSlotIndex);
}

bool AAOChest::TransferInventorySlotToChestSlot(UAOInventoryComponent* ExternalInventory, int32 ExternalSlotIndex, int32 ChestSlotIndex)
{
	if (!ChestInventory || !ExternalInventory)
	{
		return false;
	}

	if (!CanInventoryAccessChest(ExternalInventory))
	{
		return false;
	}

	return UAOInventoryComponent::ExecuteExchangeRequest(ExternalInventory, ExternalSlotIndex, ChestInventory, ChestSlotIndex);
}

bool AAOChest::TransferChestSlotToInventoryComponent(
	UAOInventoryComponent* TargetInventoryComponent,
	int32 ChestSlotIndex,
	int32 TargetSlotIndex)
{
	if (!ChestInventory || !TargetInventoryComponent)
	{
		return false;
	}

	return UAOInventoryComponent::ExecuteExchangeRequest(ChestInventory, ChestSlotIndex, TargetInventoryComponent, TargetSlotIndex);
}

bool AAOChest::TransferItemToInteractorInventory(APawn* InteractingPawn, int32 ChestSlotIndex, int32 TargetSlotIndex)
{
	if (!HasAuthority() || !InteractingPawn || !ChestInventory)
	{
		return false;
	}

	const FAOInventoryEntry* ChestEntry = ChestInventory->GetInventoryEntryAtSlot(ChestSlotIndex);
	if (ChestEntry == nullptr || ChestEntry->Instance == nullptr || ChestEntry->StackCount <= 0)
	{
		return false;
	}

	TArray<UAOInventoryComponent*> InventoryComponents;
	UAOInventoryStatics::AppendInventoryComponentsFromActor(InteractingPawn, InventoryComponents);
	if (InventoryComponents.IsEmpty())
	{
		return false;
	}

	FAOInventoryReceiveBatch ReceiveBatch;
	FAOInventoryInstanceEntry& InstanceEntry = ReceiveBatch.InstanceEntries.AddDefaulted_GetRef();
	InstanceEntry.Count = 1;
	InstanceEntry.ItemInstance = ChestEntry->Instance;

	for (UAOInventoryComponent* InventoryComponent : InventoryComponents)
	{
		if (InventoryComponent == nullptr || !InventoryComponent->CanFullyAcceptInventoryBatch(ReceiveBatch))
		{
			continue;
		}

		const int32 ResolvedTargetSlotIndex = InventoryComponent->FindAvaliableSlot(ChestEntry->Instance, 1);
		if (ResolvedTargetSlotIndex == INDEX_NONE)
		{
			continue;
		}

		return TransferChestSlotToInventoryComponent(InventoryComponent, ChestSlotIndex, ResolvedTargetSlotIndex);
	}

	return false;
}

UAOInventoryComponent* AAOChest::GetInventoryComponent()
{
	return ChestInventory;
}

const FInteractionOption* AAOChest::FindInteractionOptionByIndex(int32 InteractionOptionIndex) const
{
	if (!InteractionOptions.IsValidIndex(InteractionOptionIndex))
	{
		return nullptr;
	}

	return &InteractionOptions[InteractionOptionIndex];
}
