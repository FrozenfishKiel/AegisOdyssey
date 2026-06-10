#include "AOCombatFeedbackBlueprintLibrary.h"

#include "AegisOdyssey/UI/AOHUD.h"
#include "AegisOdyssey/UI/AOHUDViewModelComponent.h"
#include "MVVM_CombatFeedbackFeed.h"
#include "MVVM_CombatResources.h"
#include "MVVM_Crafting.h"
#include "MVVM_HUD.h"
#include "MVVM_ItemHoverTooltip.h"
#include "MVVM_LocalCombatState.h"
#include "MVVM_TargetHealthBarCollection.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCombatFeedbackBlueprintLibrary)

UMVVM_HUD* UAOCombatFeedbackBlueprintLibrary::GetMainHUDViewModel(const UObject* WorldContextObject)
{
	if (WorldContextObject == nullptr)
	{
		return nullptr;
	}

	if (const UWorld* World = WorldContextObject->GetWorld())
	{
		if (const ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController())
		{
			if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(World))
			{
				if (UAOHUDViewModelComponent* HUDViewModelComponent = AAOHUD::FindHUDOwnedComponent<UAOHUDViewModelComponent>(PlayerController))
				{
					return HUDViewModelComponent->GetHUDMVVM();
				}
			}
		}
	}

	return nullptr;
}

UMVVM_CombatResources* UAOCombatFeedbackBlueprintLibrary::GetCombatResourcesViewModel(const UObject* WorldContextObject)
{
	if (UMVVM_HUD* HUDViewModel = GetMainHUDViewModel(WorldContextObject))
	{
		return HUDViewModel->GetCombatResourcesViewModel();
	}

	return nullptr;
}

UMVVM_LocalCombatState* UAOCombatFeedbackBlueprintLibrary::GetLocalCombatStateViewModel(const UObject* WorldContextObject)
{
	if (UMVVM_HUD* HUDViewModel = GetMainHUDViewModel(WorldContextObject))
	{
		return HUDViewModel->GetLocalCombatStateViewModel();
	}

	return nullptr;
}

UMVVM_CombatFeedbackFeed* UAOCombatFeedbackBlueprintLibrary::GetCombatFeedbackFeedViewModel(const UObject* WorldContextObject)
{
	if (UMVVM_HUD* HUDViewModel = GetMainHUDViewModel(WorldContextObject))
	{
		return HUDViewModel->GetCombatFeedbackFeedViewModel();
	}

	return nullptr;
}

UMVVM_TargetHealthBarCollection* UAOCombatFeedbackBlueprintLibrary::GetTargetHealthBarCollectionViewModel(const UObject* WorldContextObject)
{
	if (UMVVM_HUD* HUDViewModel = GetMainHUDViewModel(WorldContextObject))
	{
		return HUDViewModel->GetTargetHealthBarCollectionViewModel();
	}

	return nullptr;
}

UMVVM_Crafting* UAOCombatFeedbackBlueprintLibrary::GetCraftingViewModel(const UObject* WorldContextObject)
{
	if (UMVVM_HUD* HUDViewModel = GetMainHUDViewModel(WorldContextObject))
	{
		return HUDViewModel->GetCraftingViewModel();
	}

	return nullptr;
}

UMVVM_ItemHoverTooltip* UAOCombatFeedbackBlueprintLibrary::GetItemHoverTooltipViewModel(const UObject* WorldContextObject)
{
	if (UMVVM_HUD* HUDViewModel = GetMainHUDViewModel(WorldContextObject))
	{
		return HUDViewModel->GetItemHoverTooltipViewModel();
	}

	return nullptr;
}

bool UAOCombatFeedbackBlueprintLibrary::ShouldDisplayFloatingText(const FAOCombatFeedbackViewData& Feedback)
{
	return Feedback.bShouldDisplayFloatingText;
}

bool UAOCombatFeedbackBlueprintLibrary::IsLocalRelevantFeedback(const FAOCombatFeedbackViewData& Feedback)
{
	return Feedback.bIsLocalRelevant;
}

bool UAOCombatFeedbackBlueprintLibrary::IsLocalInstigatorFeedback(const FAOCombatFeedbackViewData& Feedback)
{
	return Feedback.bIsLocalInstigator;
}

bool UAOCombatFeedbackBlueprintLibrary::IsLocalTargetFeedback(const FAOCombatFeedbackViewData& Feedback)
{
	return Feedback.bIsLocalTarget;
}

bool UAOCombatFeedbackBlueprintLibrary::ShouldRouteToHUD(const FAOCombatFeedbackViewData& Feedback)
{
	return Feedback.bShouldEnqueueForHUD;
}

bool UAOCombatFeedbackBlueprintLibrary::ShouldRouteToWorldFloatingText(const FAOCombatFeedbackViewData& Feedback)
{
	return Feedback.bShouldEnqueueForWorldFloatingText;
}

bool UAOCombatFeedbackBlueprintLibrary::IsImportantCombatFeedback(const FAOCombatFeedbackViewData& Feedback)
{
	return Feedback.bIsImportantCombatFeedback;
}

bool UAOCombatFeedbackBlueprintLibrary::IsDamageFeedback(const FAOCombatFeedbackViewData& Feedback)
{
	return Feedback.ResultType == EAOCombatResultType::Damage && Feedback.HealthDamage > KINDA_SMALL_NUMBER;
}

bool UAOCombatFeedbackBlueprintLibrary::IsDefensiveFeedback(const FAOCombatFeedbackViewData& Feedback)
{
	return Feedback.ResultType == EAOCombatResultType::Blocked
		|| Feedback.ResultType == EAOCombatResultType::Parry
		|| Feedback.ResultType == EAOCombatResultType::Broken
		|| Feedback.ResultType == EAOCombatResultType::Invulnerable;
}

FText UAOCombatFeedbackBlueprintLibrary::BuildRecommendedCombatText(const FAOCombatFeedbackViewData& Feedback)
{
	switch (Feedback.FloatingTextType)
	{
	case EAOCombatFloatingTextType::Critical:
		return FText::FromString(TEXT("Critical"));
	case EAOCombatFloatingTextType::Parry:
		return FText::FromString(TEXT("Parry"));
	case EAOCombatFloatingTextType::Broken:
		return FText::FromString(TEXT("Broken"));
	case EAOCombatFloatingTextType::Damage:
		if (Feedback.HealthDamage > KINDA_SMALL_NUMBER)
		{
			return FText::AsNumber(FMath::RoundToInt(Feedback.HealthDamage));
		}
		break;
	default:
		break;
	}

	return FText::GetEmpty();
}

bool UAOCombatFeedbackBlueprintLibrary::IsFeedbackRelatedToActor(const FAOCombatFeedbackViewData& Feedback, const AActor* Actor)
{
	return Actor != nullptr && (Feedback.Instigator == Actor || Feedback.Target == Actor);
}

TArray<FAOCombatFeedbackViewData> UAOCombatFeedbackBlueprintLibrary::ConsumePendingCombatFeedbackFromFeed(
	UMVVM_CombatFeedbackFeed* CombatFeedbackFeedViewModel)
{
	return CombatFeedbackFeedViewModel != nullptr
		? CombatFeedbackFeedViewModel->ConsumePendingCombatFeedbackList()
		: TArray<FAOCombatFeedbackViewData>();
}

TArray<FAOCombatFeedbackViewData> UAOCombatFeedbackBlueprintLibrary::ConsumePendingCombatFeedback(UMVVM_HUD* HUDViewModel)
{
	if (HUDViewModel != nullptr)
	{
		return ConsumePendingCombatFeedbackFromFeed(HUDViewModel->GetCombatFeedbackFeedViewModel());
	}

	return TArray<FAOCombatFeedbackViewData>();
}
