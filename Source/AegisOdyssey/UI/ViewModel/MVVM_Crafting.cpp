#include "MVVM_Crafting.h"

#include "AegisOdyssey/Crafting/Components/AOCraftingComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_Crafting)

namespace MVVMCraftingPrivate
{
	FText BuildRecipeBlockReasonText(EAOCraftingRecipeBlockReason InBlockReason)
	{
		switch (InBlockReason)
		{
		case EAOCraftingRecipeBlockReason::None:
			return FText::GetEmpty();
		case EAOCraftingRecipeBlockReason::InvalidRecipe:
			return FText::FromString(TEXT("配方无效"));
		case EAOCraftingRecipeBlockReason::Locked:
			return FText::FromString(TEXT("尚未解锁"));
		case EAOCraftingRecipeBlockReason::MissingMaterials:
			return FText::FromString(TEXT("材料不足"));
		case EAOCraftingRecipeBlockReason::QueueFull:
			return FText::FromString(TEXT("制造队列已满"));
		default:
			return FText::GetEmpty();
		}
	}
}

UMVVM_Crafting::UMVVM_Crafting(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMVVM_Crafting::SetObservedCraftingComponent(UAOCraftingComponent* InCraftingComponent)
{
	if (ObservedCraftingComponent.Get() == InCraftingComponent)
	{
		return;
	}

	UnbindObservedCraftingComponent();
	ObservedCraftingComponent = InCraftingComponent;

	if (InCraftingComponent != nullptr)
	{
		ObservedCraftingObservationChangedHandle =
			InCraftingComponent->OnCraftingObservationChanged.AddUObject(this, &ThisClass::HandleObservedCraftingObservationChanged);
	}

	RefreshObservationData();
}

UAOCraftingComponent* UMVVM_Crafting::GetObservedCraftingComponent() const
{
	return ObservedCraftingComponent.Get();
}

void UMVVM_Crafting::RefreshObservationData()
{
	RecipeList.Reset();
	QueueList.Reset();
	QueueSlotCount = 0;
	SelectedRecipeDetail = FAOCraftingRecipeDetailViewData();
	LastCraftRequestResult = FAOCraftingRequestResult();
	LastCraftRequestFeedback = FText::GetEmpty();
	bHasCraftRequestFeedback = false;

	if (UAOCraftingComponent* CraftingComponent = GetObservedCraftingComponent())
	{
		RecipeList = CraftingComponent->BuildRecipeListViewData();
		QueueList = CraftingComponent->BuildQueueViewData();
		QueueSlotCount = FMath::Max(0, CraftingComponent->GetMaxQueueSize());
		LastCraftRequestResult = CraftingComponent->GetLastCraftRequestResult();
		LastCraftRequestFeedback = LastCraftRequestResult.FeedbackText;
		bHasCraftRequestFeedback = !LastCraftRequestFeedback.IsEmpty();

		if (SelectedRecipeRowName.IsValid())
		{
			if (!CraftingComponent->BuildRecipeDetailViewData(SelectedRecipeRowName, SelectedRecipeDetail))
			{
				SelectedRecipeRowName = NAME_None;
			}
		}

		if (!SelectedRecipeRowName.IsValid() && !RecipeList.IsEmpty())
		{
			SelectedRecipeRowName = RecipeList[0].RecipeRowName;
			CraftingComponent->BuildRecipeDetailViewData(SelectedRecipeRowName, SelectedRecipeDetail);
		}
	}
	else
	{
		SelectedRecipeRowName = NAME_None;
	}

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetRecipeList);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetQueueList);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetQueueSlotCount);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetSelectedRecipeRowName);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetSelectedRecipeDetail);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetLastCraftRequestFeedback);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HasCraftRequestFeedback);
	OnCraftingObservationChanged.Broadcast();
}

void UMVVM_Crafting::SetSelectedRecipeRowName(FName InRecipeRowName)
{
	if (SelectedRecipeRowName != InRecipeRowName)
	{
		SelectedRecipeRowName = InRecipeRowName;
	}

	RefreshObservationData();
}

bool UMVVM_Crafting::RequestEnqueueRecipe(FName InRecipeRowName)
{
	return RequestCraftRecipe(InRecipeRowName, EAOCraftingRequestType::Single);
}

bool UMVVM_Crafting::RequestCraftRecipe(FName InRecipeRowName, EAOCraftingRequestType RequestType)
{
	if (!InRecipeRowName.IsValid())
	{
		return false;
	}

	if (UAOCraftingComponent* CraftingComponent = GetObservedCraftingComponent())
	{
		const bool bRequested = CraftingComponent->RequestCraftRecipe(InRecipeRowName, RequestType);
		RefreshObservationData();
		return bRequested;
	}

	return false;
}

bool UMVVM_Crafting::RequestEnqueueSelectedRecipe()
{
	return RequestEnqueueRecipe(SelectedRecipeRowName);
}

bool UMVVM_Crafting::RequestCraftSelectedRecipe(EAOCraftingRequestType RequestType)
{
	return RequestCraftRecipe(SelectedRecipeRowName, RequestType);
}

bool UMVVM_Crafting::CanEnqueueSelectedRecipe() const
{
	return SelectedRecipeDetail.bCanEnqueue && SelectedRecipeDetail.RecipeRowName.IsValid();
}

FText UMVVM_Crafting::GetRecipeBlockReasonText(EAOCraftingRecipeBlockReason InBlockReason) const
{
	return MVVMCraftingPrivate::BuildRecipeBlockReasonText(InBlockReason);
}

FText UMVVM_Crafting::GetSelectedRecipeBlockReasonText() const
{
	return GetRecipeBlockReasonText(SelectedRecipeDetail.BlockReason);
}

bool UMVVM_Crafting::GetActiveQueueEntry(FAOCraftingQueueEntryViewData& OutQueueEntry) const
{
	for (const FAOCraftingQueueEntryViewData& QueueEntry : QueueList)
	{
		if (QueueEntry.State == EAOCraftingQueueEntryViewState::Active)
		{
			OutQueueEntry = QueueEntry;
			return true;
		}
	}

	OutQueueEntry = FAOCraftingQueueEntryViewData();
	return false;
}

float UMVVM_Crafting::GetQueueEntryRemainingSeconds(const FAOCraftingQueueEntryViewData& QueueEntry) const
{
	if (QueueEntry.State != EAOCraftingQueueEntryViewState::Active)
	{
		return QueueEntry.ResolvedDurationSeconds;
	}

	if (QueueEntry.ExpectedFinishServerWorldTimeSeconds <= 0.0f)
	{
		return QueueEntry.ResolvedDurationSeconds;
	}

	return FMath::Max(0.0f, QueueEntry.ExpectedFinishServerWorldTimeSeconds - GetObservedServerWorldTimeSeconds());
}

float UMVVM_Crafting::GetQueueEntryProgressRatio(const FAOCraftingQueueEntryViewData& QueueEntry) const
{
	const float TotalDurationSeconds = FMath::Max(KINDA_SMALL_NUMBER, QueueEntry.ResolvedDurationSeconds);
	if (QueueEntry.State != EAOCraftingQueueEntryViewState::Active)
	{
		return 0.0f;
	}

	const float RemainingSeconds = GetQueueEntryRemainingSeconds(QueueEntry);
	return FMath::Clamp(1.0f - (RemainingSeconds / TotalDurationSeconds), 0.0f, 1.0f);
}

float UMVVM_Crafting::GetActiveQueueRemainingSeconds() const
{
	FAOCraftingQueueEntryViewData ActiveQueueEntry;
	return GetActiveQueueEntry(ActiveQueueEntry)
		? GetQueueEntryRemainingSeconds(ActiveQueueEntry)
		: 0.0f;
}

float UMVVM_Crafting::GetActiveQueueProgressRatio() const
{
	FAOCraftingQueueEntryViewData ActiveQueueEntry;
	return GetActiveQueueEntry(ActiveQueueEntry)
		? GetQueueEntryProgressRatio(ActiveQueueEntry)
		: 0.0f;
}

void UMVVM_Crafting::HandleObservedCraftingObservationChanged()
{
	RefreshObservationData();
}

void UMVVM_Crafting::UnbindObservedCraftingComponent()
{
	if (UAOCraftingComponent* CraftingComponent = ObservedCraftingComponent.Get())
	{
		if (ObservedCraftingObservationChangedHandle.IsValid())
		{
			CraftingComponent->OnCraftingObservationChanged.Remove(ObservedCraftingObservationChangedHandle);
		}
	}

	ObservedCraftingObservationChangedHandle.Reset();
	ObservedCraftingComponent.Reset();
}

float UMVVM_Crafting::GetObservedServerWorldTimeSeconds() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return 0.0f;
	}

	if (const AGameStateBase* GameState = World->GetGameState())
	{
		return GameState->GetServerWorldTimeSeconds();
	}

	return World->GetTimeSeconds();
}
