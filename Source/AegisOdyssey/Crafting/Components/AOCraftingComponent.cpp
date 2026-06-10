#include "AegisOdyssey/Crafting/Components/AOCraftingComponent.h"

#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOHealthAttributeSet.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "AegisOdyssey/Character/AOPawnData.h"
#include "AegisOdyssey/Interaction/PickUpable.h"
#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AegisOdyssey/Inventory/AOInventoryStatics.h"
#include "AegisOdyssey/Inventory/Fragments/AOFragment_InventoryIcon.h"
#include "AegisOdyssey/Inventory/InventoryInterface.h"
#include "AegisOdyssey/Items/AOItem.h"
#include "AegisOdyssey/Items/AOItemCatalogTypes.h"
#include "AegisOdyssey/System/AOGameData.h"
#include "AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryItemContextMenu.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCraftingComponent)

namespace AOCraftingComponentPrivate
{
	constexpr float MinimumCraftDurationSeconds = 0.01f;
	constexpr float FailedDeliveryRetryDelaySeconds = 1.0f;

	struct FRecipeListBuildEntry
	{
		FAOCraftingRecipeListEntryViewData ViewData;
		int32 SortOrder = 0;
		int32 DiscoveryOrder = 0;
	};

	void AddOrMergeRequiredItem(TArray<FAOCraftingItemCount>& InOutRequirements, const FAOCraftingItemCount& Requirement)
	{
		if (Requirement.ItemId == INDEX_NONE || Requirement.Count <= 0)
		{
			return;
		}

		for (FAOCraftingItemCount& ExistingRequirement : InOutRequirements)
		{
			if (ExistingRequirement.ItemId == Requirement.ItemId)
			{
				ExistingRequirement.Count += Requirement.Count;
				return;
			}
		}

		InOutRequirements.Add(Requirement);
	}

	FText BuildMissingMaterialsText()
	{
		return FText::FromString(TEXT("无法制作，材料不足"));
	}

	FText BuildQueueFullText()
	{
		return FText::FromString(TEXT("制造队列已满"));
	}

	FText BuildLockedText()
	{
		return FText::FromString(TEXT("尚未解锁"));
	}

	FText BuildInvalidRecipeText()
	{
		return FText::FromString(TEXT("配方无效"));
	}

	FText BuildStartedCraftingText(int32 RequestedCraftCount, int32 ActualCraftCount)
	{
		if (ActualCraftCount <= 0)
		{
			return BuildMissingMaterialsText();
		}

		if (RequestedCraftCount > 0 && RequestedCraftCount != ActualCraftCount)
		{
			return FText::Format(
				FText::FromString(TEXT("已开始制作 {0} 个（本次最多可制作 {0} 个）")),
				FText::AsNumber(ActualCraftCount));
		}

		return FText::Format(
			FText::FromString(TEXT("已开始制作 {0} 个")),
			FText::AsNumber(ActualCraftCount));
	}
}

UAOCraftingComponent::UAOCraftingComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	DroppedItemActorClass = AAOItem::StaticClass();
}

bool UAOCraftingComponent::RequestEnqueueRecipe(FName RecipeRowName)
{
	return RequestCraftRecipe(RecipeRowName, EAOCraftingRequestType::Single);
}

bool UAOCraftingComponent::RequestCraftRecipe(FName RecipeRowName, EAOCraftingRequestType RequestType)
{
	if (!RecipeRowName.IsValid())
	{
		UpdateLastCraftRequestResult(BuildCraftRequestFailureResult(
			RecipeRowName,
			RequestType,
			EAOCraftingRecipeBlockReason::InvalidRecipe));
		return false;
	}

	if (GetOwner() != nullptr && GetOwner()->HasAuthority())
	{
		const FAOCraftingRequestResult RequestResult = TryRequestCraftRecipeOnAuthority(RecipeRowName, RequestType);
		UpdateLastCraftRequestResult(RequestResult);
		return RequestResult.bAccepted;
	}

	ServerRequestCraftRecipe(RecipeRowName, RequestType);
	return true;
}

UMVVM_InventoryItemContextMenu* UAOCraftingComponent::GetOrCreateCraftingContextMenuViewModel()
{
	if (CraftingContextMenuViewModel == nullptr)
	{
		// 制造右键菜单的 ViewModel 直接挂在当前制造源上，
		// 这样未来切到工作台、熔炉等非玩家制造源时，也不用再借道库存来源找宿主。
		CraftingContextMenuViewModel = NewObject<UMVVM_InventoryItemContextMenu>(this);
	}

	return CraftingContextMenuViewModel;
}

TArray<FAOCraftingRecipeListEntryViewData> UAOCraftingComponent::BuildRecipeListViewData() const
{
	TArray<FAOCraftingRecipeListEntryViewData> Result;

	const UDataTable* RecipeDataTable = GetOwnerCraftingRecipeTable();
	if (RecipeDataTable == nullptr || RecipeDataTable->GetRowStruct() != FAOCraftingRecipeRow::StaticStruct())
	{
		return Result;
	}

	int32 CharacterLevel = 1;
	if (const AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(GetOwner()))
	{
		CharacterLevel = OwnerCharacter->GetCharacterLevel();
	}

	TArray<AOCraftingComponentPrivate::FRecipeListBuildEntry> PendingEntries;
	int32 DiscoveryOrder = 0;
	for (const TPair<FName, uint8*>& RowPair : RecipeDataTable->GetRowMap())
	{
		const FAOCraftingRecipeRow* RecipeRow = reinterpret_cast<const FAOCraftingRecipeRow*>(RowPair.Value);
		if (RecipeRow == nullptr || !RowPair.Key.IsValid())
		{
			continue;
		}

		const bool bUnlocked = CharacterLevel >= RecipeRow->UnlockLevel;
		if (!bUnlocked && !RecipeRow->bVisibleBeforeUnlock)
		{
			continue;
		}

		float ResolvedDurationSeconds = 0.0f;
		const EAOCraftingRecipeBlockReason BlockReason = ResolveRecipeBlockReason(RowPair.Key, RecipeRow, ResolvedDurationSeconds);

		AOCraftingComponentPrivate::FRecipeListBuildEntry& PendingEntry = PendingEntries.AddDefaulted_GetRef();
		PendingEntry.SortOrder = RecipeRow->SortOrder;
		PendingEntry.DiscoveryOrder = DiscoveryOrder++;

		FAOCraftingRecipeListEntryViewData& Entry = PendingEntry.ViewData;
		Entry.RecipeRowName = RowPair.Key;
		Entry.PrimaryOutputDefinition = ResolvePrimaryOutputDefinition(RecipeRow);
		Entry.ResolvedDurationSeconds = ResolvedDurationSeconds;
		Entry.bCanEnqueue = BlockReason == EAOCraftingRecipeBlockReason::None;
		Entry.BlockReason = BlockReason;
	}

	PendingEntries.Sort([](const AOCraftingComponentPrivate::FRecipeListBuildEntry& Left, const AOCraftingComponentPrivate::FRecipeListBuildEntry& Right)
	{
		if (Left.SortOrder != Right.SortOrder)
		{
			return Left.SortOrder < Right.SortOrder;
		}

		return Left.DiscoveryOrder < Right.DiscoveryOrder;
	});

	Result.Reserve(PendingEntries.Num());
	for (const AOCraftingComponentPrivate::FRecipeListBuildEntry& PendingEntry : PendingEntries)
	{
		Result.Add(PendingEntry.ViewData);
	}

	return Result;
}

bool UAOCraftingComponent::BuildRecipeDetailViewData(FName RecipeRowName, FAOCraftingRecipeDetailViewData& OutViewData) const
{
	OutViewData = FAOCraftingRecipeDetailViewData();

	const FAOCraftingRecipeRow* RecipeRow = nullptr;
	if (!FindRecipeRow(RecipeRowName, RecipeRow))
	{
		return false;
	}

	float ResolvedDurationSeconds = 0.0f;
	const EAOCraftingRecipeBlockReason BlockReason = ResolveRecipeBlockReason(RecipeRowName, RecipeRow, ResolvedDurationSeconds);

	OutViewData.RecipeRowName = RecipeRowName;
	OutViewData.PrimaryOutputDefinition = ResolvePrimaryOutputDefinition(RecipeRow);
	OutViewData.ResolvedDurationSeconds = ResolvedDurationSeconds;
	OutViewData.bCanEnqueue = BlockReason == EAOCraftingRecipeBlockReason::None;
	OutViewData.BlockReason = BlockReason;

	for (const FAOCraftingItemCount& MaterialEntry : RecipeRow->MaterialEntries)
	{
		FAOCraftingMaterialViewData& ViewData = OutViewData.MaterialEntries.AddDefaulted_GetRef();
		ViewData.ItemId = MaterialEntry.ItemId;
		ViewData.ItemDefinition = const_cast<UAOInventoryItemDefinition*>(FindItemDefinitionByItemId(MaterialEntry.ItemId));
		ViewData.RequiredCount = MaterialEntry.Count;
		ViewData.OwnedCount = CountAvailableMaterialByItemId(MaterialEntry.ItemId);
		ViewData.bSatisfied = ViewData.OwnedCount >= ViewData.RequiredCount;
	}

	for (const FAOCraftingItemCount& OutputEntry : RecipeRow->OutputEntries)
	{
		FAOCraftingOutputViewData& ViewData = OutViewData.OutputEntries.AddDefaulted_GetRef();
		ViewData.ItemId = OutputEntry.ItemId;
		ViewData.ItemDefinition = const_cast<UAOInventoryItemDefinition*>(FindItemDefinitionByItemId(OutputEntry.ItemId));
		ViewData.Count = OutputEntry.Count;
	}

	return true;
}

TArray<FAOCraftingQueueEntryViewData> UAOCraftingComponent::BuildQueueViewData() const
{
	TArray<FAOCraftingQueueEntryViewData> Result;
	Result.Reserve(CraftingQueue.Num());

	for (const FAOCraftingQueueEntry& QueueEntry : CraftingQueue)
	{
		const FAOCraftingRecipeRow* RecipeRow = nullptr;
		FindRecipeRow(QueueEntry.RecipeRowName, RecipeRow);
		float ResolvedDurationSeconds = QueueEntry.ResolvedDurationSeconds;
		const EAOCraftingRecipeBlockReason BlockReason =
			ResolveRecipeBlockReason(QueueEntry.RecipeRowName, RecipeRow, ResolvedDurationSeconds);

		FAOCraftingQueueEntryViewData& ViewData = Result.AddDefaulted_GetRef();
		ViewData.QueueEntryId = QueueEntry.QueueEntryId;
		ViewData.RecipeRowName = QueueEntry.RecipeRowName;
		ViewData.PrimaryOutputDefinition = ResolvePrimaryOutputDefinition(QueueEntry.OutputEntries);
		ViewData.BlockReason = BlockReason;
		ViewData.State = QueueEntry.State == EAOCraftingQueueEntryState::Active
			? EAOCraftingQueueEntryViewState::Active
			: EAOCraftingQueueEntryViewState::Queued;
		ViewData.ResolvedDurationSeconds = QueueEntry.ResolvedDurationSeconds;
		ViewData.StartServerWorldTimeSeconds = QueueEntry.StartServerWorldTimeSeconds;
		ViewData.ExpectedFinishServerWorldTimeSeconds = QueueEntry.ExpectedFinishServerWorldTimeSeconds;
		ViewData.TotalCraftCount = QueueEntry.TotalCraftCount;
		ViewData.RemainingCraftCount = QueueEntry.RemainingCraftCount;
		ViewData.CompletedCraftCount = FMath::Max(0, QueueEntry.TotalCraftCount - QueueEntry.RemainingCraftCount);
	}

	return Result;
}

void UAOCraftingComponent::HandleOwnerRuntimeInterrupted()
{
	if (GetOwner() == nullptr || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ActiveCraftingTimerHandle);
	}

	CraftingQueue.Reset();
	NotifyCraftingObservationChanged();
}

void UAOCraftingComponent::OnRegister()
{
	Super::OnRegister();

	if (APawn* OwnerPawn = GetPawn<APawn>())
	{
		if (UAOExtPawnComponent* ExtPawnComponent = UAOExtPawnComponent::FindAOExtPawnComponent(OwnerPawn))
		{
			ExtPawnComponent->CallRegister_OnASCWasAssign(FOnASCWasAssign::FDelegate::CreateUObject(this, &ThisClass::TryBindOwnerOutOfHealthDelegate));
		}
	}
}

void UAOCraftingComponent::BeginPlay()
{
	Super::BeginPlay();

	TryBindOwnerOutOfHealthDelegate();
	BindObservedInventorySources();
}

void UAOCraftingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindObservedInventorySources();

	if (BoundHealthAttributeSet.IsValid())
	{
		const_cast<UAOHealthAttributeSet*>(BoundHealthAttributeSet.Get())->OnOutOfHealth.RemoveAll(this);
		BoundHealthAttributeSet.Reset();
	}

	if (GetOwner() != nullptr && GetOwner()->HasAuthority())
	{
		HandleOwnerRuntimeInterrupted();
	}

	Super::EndPlay(EndPlayReason);
}

void UAOCraftingComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, CraftingQueue, COND_OwnerOnly, REPNOTIFY_Always);
}

void UAOCraftingComponent::OnRep_CraftingQueue()
{
	NotifyCraftingObservationChanged();
}

void UAOCraftingComponent::ServerRequestEnqueueRecipe_Implementation(FName RecipeRowName)
{
	const FAOCraftingRequestResult RequestResult = TryRequestCraftRecipeOnAuthority(RecipeRowName, EAOCraftingRequestType::Single);
	UpdateLastCraftRequestResult(RequestResult);
	ClientNotifyCraftRequestResult(RequestResult);
}

void UAOCraftingComponent::ServerRequestCraftRecipe_Implementation(FName RecipeRowName, EAOCraftingRequestType RequestType)
{
	const FAOCraftingRequestResult RequestResult = TryRequestCraftRecipeOnAuthority(RecipeRowName, RequestType);
	UpdateLastCraftRequestResult(RequestResult);
	ClientNotifyCraftRequestResult(RequestResult);
}

void UAOCraftingComponent::ClientNotifyCraftRequestResult_Implementation(const FAOCraftingRequestResult& RequestResult)
{
	UpdateLastCraftRequestResult(RequestResult);
}

FAOCraftingRequestResult UAOCraftingComponent::TryRequestCraftRecipeOnAuthority(FName RecipeRowName, EAOCraftingRequestType RequestType)
{
	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr || !OwnerActor->HasAuthority())
	{
		return BuildCraftRequestFailureResult(RecipeRowName, RequestType, EAOCraftingRecipeBlockReason::InvalidRecipe);
	}

	if (CraftingQueue.Num() >= MaxQueueSize)
	{
		return BuildCraftRequestFailureResult(RecipeRowName, RequestType, EAOCraftingRecipeBlockReason::QueueFull);
	}

	if (!IsRecipeUnlockedForOwner(RecipeRowName))
	{
		return BuildCraftRequestFailureResult(RecipeRowName, RequestType, EAOCraftingRecipeBlockReason::Locked);
	}

	FResolvedCraftingRecipeRuntimeData RuntimeData;
	if (!ResolveRecipeRuntimeData(RecipeRowName, RuntimeData) || RuntimeData.RecipeRow == nullptr)
	{
		return BuildCraftRequestFailureResult(RecipeRowName, RequestType, EAOCraftingRecipeBlockReason::InvalidRecipe);
	}

	const int32 MaxCraftableCount = CountMaxCraftableCount(*RuntimeData.RecipeRow);
	const int32 ActualCraftCount = ResolveActualCraftCount(RequestType, MaxCraftableCount);
	if (ActualCraftCount <= 0)
	{
		return BuildCraftRequestFailureResult(RecipeRowName, RequestType, EAOCraftingRecipeBlockReason::MissingMaterials);
	}

	const int32 RequestedCraftCount =
		RequestType == EAOCraftingRequestType::All
			? MaxCraftableCount
			: (RequestType == EAOCraftingRequestType::Ten ? 10 : 1);

	TArray<FAOCraftingMaterialConsumePlanEntry> ConsumePlan;
	if (!BuildMaterialConsumePlan(*RuntimeData.RecipeRow, ActualCraftCount, ConsumePlan))
	{
		return BuildCraftRequestFailureResult(RecipeRowName, RequestType, EAOCraftingRecipeBlockReason::MissingMaterials);
	}

	int32 ConsumedPlanCount = 0;
	if (!ExecuteMaterialConsumePlan(ConsumePlan, ConsumedPlanCount))
	{
		RollbackMaterialConsumePlan(ConsumePlan, ConsumedPlanCount);
		return BuildCraftRequestFailureResult(RecipeRowName, RequestType, EAOCraftingRecipeBlockReason::MissingMaterials);
	}

	FAOCraftingQueueEntry& NewEntry = CraftingQueue.AddDefaulted_GetRef();
	NewEntry.QueueEntryId = NextQueueEntryId++;
	NewEntry.RecipeRowName = RecipeRowName;
	NewEntry.State = EAOCraftingQueueEntryState::Queued;
	NewEntry.ResolvedDurationSeconds = RuntimeData.ResolvedDurationSeconds;
	NewEntry.TotalCraftCount = ActualCraftCount;
	NewEntry.RemainingCraftCount = ActualCraftCount;
	NewEntry.OutputEntries = RuntimeData.ResolvedOutputs;

	NotifyCraftingObservationChanged();
	RefreshActiveCraftingTimer();
	return BuildCraftRequestSuccessResult(RecipeRowName, RequestType, RequestedCraftCount, ActualCraftCount);
}

bool UAOCraftingComponent::TryEnqueueRecipeOnAuthority(FName RecipeRowName)
{
	return TryRequestCraftRecipeOnAuthority(RecipeRowName, EAOCraftingRequestType::Single).bAccepted;
}

bool UAOCraftingComponent::IsRecipeUnlockedForOwner(FName RecipeRowName) const
{
	int32 CharacterLevel = 1;
	if (const AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(GetOwner()))
	{
		CharacterLevel = OwnerCharacter->GetCharacterLevel();
	}

	const FAOCraftingRecipeRow* RecipeRow = nullptr;
	if (!FindRecipeRow(RecipeRowName, RecipeRow) || RecipeRow == nullptr)
	{
		return false;
	}

	return CharacterLevel >= RecipeRow->UnlockLevel;
}

bool UAOCraftingComponent::ResolveRecipeRuntimeData(FName RecipeRowName, FResolvedCraftingRecipeRuntimeData& OutRuntimeData) const
{
	OutRuntimeData = FResolvedCraftingRecipeRuntimeData();

	const FAOCraftingRecipeRow* RecipeRow = nullptr;
	if (!FindRecipeRow(RecipeRowName, RecipeRow) || RecipeRow == nullptr)
	{
		return false;
	}

	OutRuntimeData.RecipeRow = RecipeRow;
	OutRuntimeData.ResolvedDurationSeconds = ResolveCraftingDurationSeconds(*RecipeRow);

	for (const FAOCraftingItemCount& OutputEntry : RecipeRow->OutputEntries)
	{
		const FAOItemCatalogRow* ItemCatalogRow = UAOGameData::Get().FindItemCatalogRowById(OutputEntry.ItemId);
		if (OutputEntry.ItemId == INDEX_NONE || OutputEntry.Count <= 0 || ItemCatalogRow == nullptr || ItemCatalogRow->ItemDefinitionClass == nullptr)
		{
			return false;
		}

		FAOCraftingResolvedItemEntry& ResolvedOutput = OutRuntimeData.ResolvedOutputs.AddDefaulted_GetRef();
		ResolvedOutput.ItemId = OutputEntry.ItemId;
		ResolvedOutput.Count = OutputEntry.Count;
		ResolvedOutput.ItemDefinitionClass = ItemCatalogRow->ItemDefinitionClass;
	}

	return !OutRuntimeData.ResolvedOutputs.IsEmpty();
}

bool UAOCraftingComponent::BuildOutputReceiveBatch(const TArray<FAOCraftingResolvedItemEntry>& OutputEntries, FAOInventoryReceiveBatch& OutReceiveBatch) const
{
	OutReceiveBatch = FAOInventoryReceiveBatch();

	for (const FAOCraftingResolvedItemEntry& OutputEntry : OutputEntries)
	{
		if (OutputEntry.ItemDefinitionClass == nullptr || OutputEntry.Count <= 0)
		{
			return false;
		}

		FAOInventoryDefinitionEntry& DefinitionEntry = OutReceiveBatch.DefinitionEntries.AddDefaulted_GetRef();
		DefinitionEntry.Count = OutputEntry.Count;
		DefinitionEntry.ItemDefinitionClass = OutputEntry.ItemDefinitionClass;
	}

	return !OutReceiveBatch.IsEmpty();
}

bool UAOCraftingComponent::BuildMaterialConsumePlan(
	const FAOCraftingRecipeRow& RecipeRow,
	int32 CraftCount,
	TArray<FAOCraftingMaterialConsumePlanEntry>& OutConsumePlan) const
{
	OutConsumePlan.Reset();
	if (CraftCount <= 0)
	{
		return false;
	}

	TArray<FAOCraftingItemCount> AggregatedRequirements;
	for (const FAOCraftingItemCount& MaterialEntry : RecipeRow.MaterialEntries)
	{
		FAOCraftingItemCount ScaledRequirement = MaterialEntry;
		ScaledRequirement.Count *= CraftCount;
		AOCraftingComponentPrivate::AddOrMergeRequiredItem(AggregatedRequirements, ScaledRequirement);
	}

	TArray<UAOInventoryComponent*> InventoryComponents;
	CollectOwnerInventoryComponentsInRegistrationOrder(InventoryComponents);

	TArray<FInventorySlotSnapshot> SlotSnapshots;
	for (UAOInventoryComponent* InventoryComponent : InventoryComponents)
	{
		if (InventoryComponent == nullptr)
		{
			continue;
		}

		const TArray<FAOInventoryEntry> InventoryEntries = InventoryComponent->GetAllLists();
		for (int32 SlotIndex = 0; SlotIndex < InventoryEntries.Num(); ++SlotIndex)
		{
			const FAOInventoryEntry& InventoryEntry = InventoryEntries[SlotIndex];
			if (InventoryEntry.Instance == nullptr || InventoryEntry.StackCount <= 0)
			{
				continue;
			}

			FInventorySlotSnapshot& SlotSnapshot = SlotSnapshots.AddDefaulted_GetRef();
			SlotSnapshot.InventoryComponent = InventoryComponent;
			SlotSnapshot.SlotIndex = SlotIndex;
			SlotSnapshot.RemainingCount = InventoryEntry.StackCount;
		}
	}

	for (const FAOCraftingItemCount& Requirement : AggregatedRequirements)
	{
		int32 RemainingRequiredCount = Requirement.Count;
		for (FInventorySlotSnapshot& SlotSnapshot : SlotSnapshots)
		{
			if (RemainingRequiredCount <= 0 || SlotSnapshot.InventoryComponent == nullptr || SlotSnapshot.RemainingCount <= 0)
			{
				continue;
			}

			const TArray<FAOInventoryEntry> InventoryEntries = SlotSnapshot.InventoryComponent->GetAllLists();
			if (!InventoryEntries.IsValidIndex(SlotSnapshot.SlotIndex))
			{
				continue;
			}

			const FAOInventoryEntry& InventoryEntry = InventoryEntries[SlotSnapshot.SlotIndex];
			if (InventoryEntry.Instance == nullptr)
			{
				continue;
			}

			const FAOItemCatalogRow* InventoryItemCatalogRow = UAOGameData::Get().FindItemCatalogRowByDefinitionClass(InventoryEntry.Instance->ItemDef);
			if (InventoryItemCatalogRow == nullptr || InventoryItemCatalogRow->ItemId != Requirement.ItemId)
			{
				continue;
			}

			const int32 ConsumedCount = FMath::Min(RemainingRequiredCount, SlotSnapshot.RemainingCount);
			if (ConsumedCount <= 0)
			{
				continue;
			}

			FAOCraftingMaterialConsumePlanEntry& ConsumePlanEntry = OutConsumePlan.AddDefaulted_GetRef();
			ConsumePlanEntry.InventoryComponent = SlotSnapshot.InventoryComponent;
			ConsumePlanEntry.SlotIndex = SlotSnapshot.SlotIndex;
			ConsumePlanEntry.ItemId = Requirement.ItemId;
			ConsumePlanEntry.Count = ConsumedCount;

			SlotSnapshot.RemainingCount -= ConsumedCount;
			RemainingRequiredCount -= ConsumedCount;
		}

		if (RemainingRequiredCount > 0)
		{
			OutConsumePlan.Reset();
			return false;
		}
	}

	return true;
}

int32 UAOCraftingComponent::CountMaxCraftableCount(const FAOCraftingRecipeRow& RecipeRow) const
{
	int32 MaxCraftableCount = MAX_int32;
	bool bHasValidRequirement = false;

	for (const FAOCraftingItemCount& MaterialEntry : RecipeRow.MaterialEntries)
	{
		if (MaterialEntry.ItemId == INDEX_NONE || MaterialEntry.Count <= 0)
		{
			continue;
		}

		bHasValidRequirement = true;
		const int32 OwnedCount = CountAvailableMaterialByItemId(MaterialEntry.ItemId);
		MaxCraftableCount = FMath::Min(MaxCraftableCount, OwnedCount / MaterialEntry.Count);
	}

	if (!bHasValidRequirement)
	{
		return 0;
	}

	return FMath::Max(0, MaxCraftableCount);
}

int32 UAOCraftingComponent::ResolveActualCraftCount(EAOCraftingRequestType RequestType, int32 MaxCraftableCount) const
{
	if (MaxCraftableCount <= 0)
	{
		return 0;
	}

	switch (RequestType)
	{
	case EAOCraftingRequestType::Single:
		return FMath::Min(1, MaxCraftableCount);
	case EAOCraftingRequestType::Ten:
		return FMath::Min(10, MaxCraftableCount);
	case EAOCraftingRequestType::All:
		return MaxCraftableCount;
	default:
		return 0;
	}
}

FAOCraftingRequestResult UAOCraftingComponent::BuildCraftRequestFailureResult(
	FName RecipeRowName,
	EAOCraftingRequestType RequestType,
	EAOCraftingRecipeBlockReason FailureReason) const
{
	FAOCraftingRequestResult RequestResult;
	RequestResult.RecipeRowName = RecipeRowName;
	RequestResult.RequestType = RequestType;
	RequestResult.FailureReason = FailureReason;

	switch (FailureReason)
	{
	case EAOCraftingRecipeBlockReason::MissingMaterials:
		RequestResult.FeedbackText = AOCraftingComponentPrivate::BuildMissingMaterialsText();
		break;
	case EAOCraftingRecipeBlockReason::QueueFull:
		RequestResult.FeedbackText = AOCraftingComponentPrivate::BuildQueueFullText();
		break;
	case EAOCraftingRecipeBlockReason::Locked:
		RequestResult.FeedbackText = AOCraftingComponentPrivate::BuildLockedText();
		break;
	case EAOCraftingRecipeBlockReason::InvalidRecipe:
	default:
		RequestResult.FeedbackText = AOCraftingComponentPrivate::BuildInvalidRecipeText();
		break;
	}

	return RequestResult;
}

FAOCraftingRequestResult UAOCraftingComponent::BuildCraftRequestSuccessResult(
	FName RecipeRowName,
	EAOCraftingRequestType RequestType,
	int32 RequestedCraftCount,
	int32 ActualCraftCount) const
{
	FAOCraftingRequestResult RequestResult;
	RequestResult.RecipeRowName = RecipeRowName;
	RequestResult.RequestType = RequestType;
	RequestResult.bAccepted = ActualCraftCount > 0;
	RequestResult.RequestedCraftCount = RequestedCraftCount;
	RequestResult.ActualCraftCount = ActualCraftCount;
	RequestResult.FailureReason = EAOCraftingRecipeBlockReason::None;
	RequestResult.FeedbackText = AOCraftingComponentPrivate::BuildStartedCraftingText(RequestedCraftCount, ActualCraftCount);
	return RequestResult;
}

bool UAOCraftingComponent::ExecuteMaterialConsumePlan(const TArray<FAOCraftingMaterialConsumePlanEntry>& ConsumePlan, int32& OutConsumedPlanCount)
{
	OutConsumedPlanCount = 0;

	for (const FAOCraftingMaterialConsumePlanEntry& ConsumePlanEntry : ConsumePlan)
	{
		if (ConsumePlanEntry.InventoryComponent == nullptr || !ConsumePlanEntry.InventoryComponent->ConsumeItemAtSlot(ConsumePlanEntry.SlotIndex, ConsumePlanEntry.Count))
		{
			return false;
		}

		++OutConsumedPlanCount;
	}

	return true;
}

void UAOCraftingComponent::RollbackMaterialConsumePlan(const TArray<FAOCraftingMaterialConsumePlanEntry>& ConsumePlan, int32 ConsumedPlanCount)
{
	for (int32 ConsumePlanIndex = FMath::Min(ConsumedPlanCount, ConsumePlan.Num()) - 1; ConsumePlanIndex >= 0; --ConsumePlanIndex)
	{
		const FAOCraftingMaterialConsumePlanEntry& ConsumePlanEntry = ConsumePlan[ConsumePlanIndex];
		if (ConsumePlanEntry.InventoryComponent == nullptr || ConsumePlanEntry.ItemId == INDEX_NONE || ConsumePlanEntry.Count <= 0)
		{
			continue;
		}

		const FAOItemCatalogRow* ItemCatalogRow = UAOGameData::Get().FindItemCatalogRowById(ConsumePlanEntry.ItemId);
		if (ItemCatalogRow == nullptr || ItemCatalogRow->ItemDefinitionClass == nullptr)
		{
			continue;
		}

		int32 RestoreCount = ConsumePlanEntry.Count;
		bool bRestored = false;
		ConsumePlanEntry.InventoryComponent->AddItemDefinition(nullptr, ItemCatalogRow->ItemDefinitionClass, RestoreCount, bRestored);
	}
}

int32 UAOCraftingComponent::CountAvailableMaterialByItemId(int32 ItemId) const
{
	if (ItemId == INDEX_NONE)
	{
		return 0;
	}

	int32 TotalCount = 0;
	TArray<UAOInventoryComponent*> InventoryComponents;
	CollectOwnerInventoryComponentsInRegistrationOrder(InventoryComponents);

	for (const UAOInventoryComponent* InventoryComponent : InventoryComponents)
	{
		if (InventoryComponent == nullptr)
		{
			continue;
		}

		for (const FAOInventoryEntry& InventoryEntry : InventoryComponent->GetAllLists())
		{
			if (InventoryEntry.Instance == nullptr || InventoryEntry.StackCount <= 0)
			{
				continue;
			}

			const FAOItemCatalogRow* ItemCatalogRow = UAOGameData::Get().FindItemCatalogRowByDefinitionClass(InventoryEntry.Instance->ItemDef);
			if (ItemCatalogRow != nullptr && ItemCatalogRow->ItemId == ItemId)
			{
				TotalCount += InventoryEntry.StackCount;
			}
		}
	}

	return TotalCount;
}

EAOCraftingRecipeBlockReason UAOCraftingComponent::ResolveRecipeBlockReason(
	FName RecipeRowName,
	const FAOCraftingRecipeRow* RecipeRow,
	float& OutResolvedDurationSeconds) const
{
	OutResolvedDurationSeconds = 0.0f;

	if (!RecipeRowName.IsValid() || RecipeRow == nullptr)
	{
		return EAOCraftingRecipeBlockReason::InvalidRecipe;
	}

	if (CraftingQueue.Num() >= MaxQueueSize)
	{
		return EAOCraftingRecipeBlockReason::QueueFull;
	}

	if (!IsRecipeUnlockedForOwner(RecipeRowName))
	{
		return EAOCraftingRecipeBlockReason::Locked;
	}

	OutResolvedDurationSeconds = ResolveCraftingDurationSeconds(*RecipeRow);

	TArray<FAOCraftingMaterialConsumePlanEntry> ConsumePlan;
	return BuildMaterialConsumePlan(*RecipeRow, 1, ConsumePlan)
		? EAOCraftingRecipeBlockReason::None
		: EAOCraftingRecipeBlockReason::MissingMaterials;
}

const UAOInventoryItemDefinition* UAOCraftingComponent::FindItemDefinitionByItemId(int32 ItemId) const
{
	const FAOItemCatalogRow* ItemCatalogRow = UAOGameData::Get().FindItemCatalogRowById(ItemId);
	if (ItemCatalogRow == nullptr || ItemCatalogRow->ItemDefinitionClass == nullptr)
	{
		return nullptr;
	}

	return ItemCatalogRow->ItemDefinitionClass->GetDefaultObject<UAOInventoryItemDefinition>();
}

UAOInventoryItemDefinition* UAOCraftingComponent::ResolvePrimaryOutputDefinition(const FAOCraftingRecipeRow* RecipeRow) const
{
	if (RecipeRow == nullptr)
	{
		return nullptr;
	}

	for (const FAOCraftingItemCount& OutputEntry : RecipeRow->OutputEntries)
	{
		if (const UAOInventoryItemDefinition* ItemDefinition = FindItemDefinitionByItemId(OutputEntry.ItemId))
		{
			return const_cast<UAOInventoryItemDefinition*>(ItemDefinition);
		}
	}

	return nullptr;
}

UAOInventoryItemDefinition* UAOCraftingComponent::ResolvePrimaryOutputDefinition(
	const TArray<FAOCraftingResolvedItemEntry>& OutputEntries) const
{
	for (const FAOCraftingResolvedItemEntry& OutputEntry : OutputEntries)
	{
		if (OutputEntry.ItemDefinitionClass != nullptr)
		{
			return OutputEntry.ItemDefinitionClass->GetDefaultObject<UAOInventoryItemDefinition>();
		}
	}

	return nullptr;
}

bool UAOCraftingComponent::FindRecipeRow(FName RecipeRowName, const FAOCraftingRecipeRow*& OutRecipeRow) const
{
	OutRecipeRow = nullptr;

	const UDataTable* CraftingRecipeDataTable = GetOwnerCraftingRecipeTable();
	if (CraftingRecipeDataTable == nullptr || CraftingRecipeDataTable->GetRowStruct() != FAOCraftingRecipeRow::StaticStruct())
	{
		return false;
	}

	OutRecipeRow = CraftingRecipeDataTable->FindRow<FAOCraftingRecipeRow>(RecipeRowName, TEXT("UAOCraftingComponent::FindRecipeRow"));
	return OutRecipeRow != nullptr;
}

bool UAOCraftingComponent::StartNextQueuedEntry()
{
	if (FindActiveCraftingEntryIndex() != INDEX_NONE)
	{
		return true;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	for (FAOCraftingQueueEntry& QueueEntry : CraftingQueue)
	{
		if (QueueEntry.State != EAOCraftingQueueEntryState::Queued)
		{
			continue;
		}

		QueueEntry.State = EAOCraftingQueueEntryState::Active;
		QueueEntry.StartServerWorldTimeSeconds = World->GetTimeSeconds();
		QueueEntry.ExpectedFinishServerWorldTimeSeconds =
			QueueEntry.StartServerWorldTimeSeconds + FMath::Max(AOCraftingComponentPrivate::MinimumCraftDurationSeconds, QueueEntry.ResolvedDurationSeconds);

		World->GetTimerManager().SetTimer(
			ActiveCraftingTimerHandle,
			this,
			&ThisClass::HandleActiveCraftingFinished,
			FMath::Max(AOCraftingComponentPrivate::MinimumCraftDurationSeconds, QueueEntry.ResolvedDurationSeconds),
			false);

		NotifyCraftingObservationChanged();
		return true;
	}

	return false;
}

void UAOCraftingComponent::HandleActiveCraftingFinished()
{
	if (GetOwner() == nullptr || !GetOwner()->HasAuthority())
	{
		return;
	}

	const int32 ActiveCraftingEntryIndex = FindActiveCraftingEntryIndex();
	if (!CraftingQueue.IsValidIndex(ActiveCraftingEntryIndex))
	{
		return;
	}

	FAOCraftingQueueEntry& ActiveEntry = CraftingQueue[ActiveCraftingEntryIndex];
	bool bDeliverySucceeded = false;
	FAOInventoryReceiveBatch OutputBatch;
	if (BuildOutputReceiveBatch(ActiveEntry.OutputEntries, OutputBatch))
	{
		if (!UAOInventoryStatics::TryAddInventoryBatchToActor(GetOwner(), OutputBatch))
		{
			const bool bDropped = DropCraftingOutputsToWorld(ActiveEntry.OutputEntries);
			ensureAlwaysMsgf(
				bDropped,
				TEXT("UAOCraftingComponent::HandleActiveCraftingFinished failed to deliver crafting outputs. Owner=%s Recipe=%s"),
				*GetNameSafe(GetOwner()),
				*ActiveEntry.RecipeRowName.ToString());
		}
		else
		{
			bDeliverySucceeded = true;
		}
	}

	if (!bDeliverySucceeded && OutputBatch.IsEmpty())
	{
		bDeliverySucceeded = DropCraftingOutputsToWorld(ActiveEntry.OutputEntries);
	}

	if (!bDeliverySucceeded && !OutputBatch.IsEmpty())
	{
		bDeliverySucceeded = DropCraftingOutputsToWorld(ActiveEntry.OutputEntries);
	}

	if (!bDeliverySucceeded)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				ActiveCraftingTimerHandle,
				this,
				&ThisClass::HandleActiveCraftingFinished,
				AOCraftingComponentPrivate::FailedDeliveryRetryDelaySeconds,
				false);
		}

		return;
	}

	ActiveEntry.RemainingCraftCount = FMath::Max(0, ActiveEntry.RemainingCraftCount - 1);
	if (ActiveEntry.RemainingCraftCount > 0)
	{
		if (UWorld* World = GetWorld())
		{
			ActiveEntry.StartServerWorldTimeSeconds = World->GetTimeSeconds();
			ActiveEntry.ExpectedFinishServerWorldTimeSeconds =
				ActiveEntry.StartServerWorldTimeSeconds + FMath::Max(AOCraftingComponentPrivate::MinimumCraftDurationSeconds, ActiveEntry.ResolvedDurationSeconds);

			World->GetTimerManager().SetTimer(
				ActiveCraftingTimerHandle,
				this,
				&ThisClass::HandleActiveCraftingFinished,
				FMath::Max(AOCraftingComponentPrivate::MinimumCraftDurationSeconds, ActiveEntry.ResolvedDurationSeconds),
				false);
		}

		NotifyCraftingObservationChanged();
		return;
	}

	CraftingQueue.RemoveAt(ActiveCraftingEntryIndex);
	NotifyCraftingObservationChanged();
	RefreshActiveCraftingTimer();
}

int32 UAOCraftingComponent::FindActiveCraftingEntryIndex() const
{
	for (int32 QueueEntryIndex = 0; QueueEntryIndex < CraftingQueue.Num(); ++QueueEntryIndex)
	{
		if (CraftingQueue[QueueEntryIndex].State == EAOCraftingQueueEntryState::Active)
		{
			return QueueEntryIndex;
		}
	}

	return INDEX_NONE;
}

void UAOCraftingComponent::NotifyCraftingObservationChanged()
{
	OnCraftingObservationChanged.Broadcast();
}

void UAOCraftingComponent::RefreshActiveCraftingTimer()
{
	if (GetOwner() == nullptr || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (FindActiveCraftingEntryIndex() == INDEX_NONE)
	{
		StartNextQueuedEntry();
	}
}

void UAOCraftingComponent::UpdateLastCraftRequestResult(const FAOCraftingRequestResult& RequestResult)
{
	LastCraftRequestResult = RequestResult;
	NotifyCraftingObservationChanged();
}

float UAOCraftingComponent::ResolveCraftingDurationSeconds(const FAOCraftingRecipeRow& RecipeRow) const
{
	const float TotalCraftingSpeedBonus = ResolveTotalCraftingSpeedBonus();
	const float DurationScale = FMath::Max(0.1f, 1.0f + TotalCraftingSpeedBonus);
	return FMath::Max(AOCraftingComponentPrivate::MinimumCraftDurationSeconds, RecipeRow.BaseCraftDurationSeconds / DurationScale);
}

float UAOCraftingComponent::ResolveTotalCraftingSpeedBonus() const
{
	if (const UAOCombatAttributeSet* CombatAttributeSet = ResolveCombatAttributeSet())
	{
		return CombatAttributeSet->GetCraftingSpeedBonus();
	}

	return 0.0f;
}

bool UAOCraftingComponent::DropCraftingOutputsToWorld(const TArray<FAOCraftingResolvedItemEntry>& OutputEntries) const
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (OwnerActor == nullptr || World == nullptr || DroppedItemActorClass == nullptr)
	{
		return false;
	}

	FInventoryPickUp DroppedInventory;
	for (const FAOCraftingResolvedItemEntry& OutputEntry : OutputEntries)
	{
		if (OutputEntry.Count <= 0 || OutputEntry.ItemDefinitionClass == nullptr)
		{
			continue;
		}

		FPickUpTemplate& PickUpTemplate = DroppedInventory.Templates.AddDefaulted_GetRef();
		PickUpTemplate.StackCount = OutputEntry.Count;
		PickUpTemplate.ItemDef = OutputEntry.ItemDefinitionClass;
	}

	if (DroppedInventory.Templates.IsEmpty())
	{
		return false;
	}

	const FVector SpawnLocation = OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * 60.0f;
	const FTransform SpawnTransform(OwnerActor->GetActorRotation(), SpawnLocation);
	AAOItem* DroppedItemActor = World->SpawnActorDeferred<AAOItem>(DroppedItemActorClass, SpawnTransform, OwnerActor);
	if (DroppedItemActor == nullptr)
	{
		return false;
	}

	DroppedItemActor->SetStaticInventory(DroppedInventory);
	DroppedItemActor->SetInteractionText(NSLOCTEXT("AOCrafting", "DroppedCraftResultInteractionText", "拾取"));
	DroppedItemActor->SetLifeSpan(DroppedCraftItemLifeSeconds);
	DroppedItemActor->FinishSpawning(SpawnTransform);
	return true;
}

const UDataTable* UAOCraftingComponent::GetOwnerCraftingRecipeTable() const
{
	const APawn* OwnerPawn = GetPawn<APawn>();
	if (OwnerPawn == nullptr)
	{
		return nullptr;
	}

	const UAOExtPawnComponent* ExtPawnComponent = UAOExtPawnComponent::FindAOExtPawnComponent(OwnerPawn);
	if (ExtPawnComponent == nullptr)
	{
		return nullptr;
	}

	const UAOPawnData* PawnData = ExtPawnComponent->GetPawnData<UAOPawnData>();
	return PawnData != nullptr ? PawnData->GetCraftingRecipeDataTable() : nullptr;
}

const UAOCombatAttributeSet* UAOCraftingComponent::ResolveCombatAttributeSet() const
{
	const APawn* OwnerPawn = GetPawn<APawn>();
	if (OwnerPawn == nullptr)
	{
		return nullptr;
	}

	const UAOExtPawnComponent* ExtPawnComponent = UAOExtPawnComponent::FindAOExtPawnComponent(OwnerPawn);
	if (ExtPawnComponent == nullptr)
	{
		return nullptr;
	}

	const UAOAbilitySystem* AbilitySystemComponent = ExtPawnComponent->GetAOAbilitySystemComponent();
	return AbilitySystemComponent != nullptr
		? Cast<UAOCombatAttributeSet>(AbilitySystemComponent->GetAttributeSet(UAOCombatAttributeSet::StaticClass()))
		: nullptr;
}

const UAOHealthAttributeSet* UAOCraftingComponent::ResolveHealthAttributeSet() const
{
	const APawn* OwnerPawn = GetPawn<APawn>();
	if (OwnerPawn == nullptr)
	{
		return nullptr;
	}

	const UAOExtPawnComponent* ExtPawnComponent = UAOExtPawnComponent::FindAOExtPawnComponent(OwnerPawn);
	if (ExtPawnComponent == nullptr)
	{
		return nullptr;
	}

	const UAOAbilitySystem* AbilitySystemComponent = ExtPawnComponent->GetAOAbilitySystemComponent();
	return AbilitySystemComponent != nullptr
		? Cast<UAOHealthAttributeSet>(AbilitySystemComponent->GetAttributeSet(UAOHealthAttributeSet::StaticClass()))
		: nullptr;
}

void UAOCraftingComponent::TryBindOwnerOutOfHealthDelegate()
{
	const UAOHealthAttributeSet* HealthAttributeSet = ResolveHealthAttributeSet();
	if (HealthAttributeSet == nullptr)
	{
		return;
	}

	if (BoundHealthAttributeSet.Get() == HealthAttributeSet)
	{
		return;
	}

	if (BoundHealthAttributeSet.IsValid())
	{
		const_cast<UAOHealthAttributeSet*>(BoundHealthAttributeSet.Get())->OnOutOfHealth.RemoveAll(this);
		BoundHealthAttributeSet.Reset();
	}

	BoundHealthAttributeSet = HealthAttributeSet;
	const_cast<UAOHealthAttributeSet*>(HealthAttributeSet)->OnOutOfHealth.AddUObject(this, &ThisClass::HandleOwnerOutOfHealth);
}

void UAOCraftingComponent::HandleOwnerOutOfHealth(
	AActor* EffectInstigator,
	AActor* EffectCauser,
	const FGameplayEffectSpec* EffectSpec,
	float EffectMagnitude,
	float OldValue,
	float NewValue)
{
	(void)EffectInstigator;
	(void)EffectCauser;
	(void)EffectSpec;
	(void)EffectMagnitude;
	(void)OldValue;
	(void)NewValue;

	HandleOwnerRuntimeInterrupted();
}

void UAOCraftingComponent::CollectOwnerInventoryComponentsInRegistrationOrder(TArray<UAOInventoryComponent*>& OutInventoryComponents) const
{
	OutInventoryComponents.Reset();

	const AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr)
	{
		return;
	}

	const TArray<UActorComponent*> InventoryComponents = OwnerActor->GetComponentsByInterface(UInventoryInterface::StaticClass());
	for (UActorComponent* ActorComponent : InventoryComponents)
	{
		TScriptInterface<IInventoryInterface> InventoryInterface(ActorComponent);
		if (!InventoryInterface)
		{
			continue;
		}

		if (UAOInventoryComponent* InventoryComponent = InventoryInterface->GetInventoryComponent())
		{
			OutInventoryComponents.Add(InventoryComponent);
		}
	}
}

void UAOCraftingComponent::BindObservedInventorySources()
{
	UnbindObservedInventorySources();

	TArray<UAOInventoryComponent*> InventoryComponents;
	CollectOwnerInventoryComponentsInRegistrationOrder(InventoryComponents);
	for (UAOInventoryComponent* InventoryComponent : InventoryComponents)
	{
		if (InventoryComponent == nullptr)
		{
			continue;
		}

		InventoryComponent->OnInventoryObservedChanged.AddUObject(this, &ThisClass::HandleObservedInventoryChanged);
		ObservedInventoryComponents.Add(InventoryComponent);
	}
}

void UAOCraftingComponent::UnbindObservedInventorySources()
{
	for (const TWeakObjectPtr<UAOInventoryComponent>& InventoryComponent : ObservedInventoryComponents)
	{
		if (InventoryComponent.IsValid())
		{
			InventoryComponent->OnInventoryObservedChanged.RemoveAll(this);
		}
	}

	ObservedInventoryComponents.Reset();
}

void UAOCraftingComponent::HandleObservedInventoryChanged()
{
	NotifyCraftingObservationChanged();
}
