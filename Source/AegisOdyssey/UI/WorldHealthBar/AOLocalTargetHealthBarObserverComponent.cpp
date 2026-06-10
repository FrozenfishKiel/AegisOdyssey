#include "AOLocalTargetHealthBarObserverComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AegisOdyssey/AOStateTags.h"
#include "AegisOdyssey/UI/ViewModel/AOCombatFeedbackViewData.h"
#include "AOTargetHealthBarComponent.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOLocalTargetHealthBarObserverComponent)

namespace AOLocalTargetHealthBarObserverComponentPrivate
{
	const FGameplayTagContainer& GetCombatDisplayTags()
	{
		static FGameplayTagContainer CombatDisplayTags;
		static bool bInitialized = false;
		if (!bInitialized)
		{
			CombatDisplayTags.AddTag(AOStateTags::State_Combat_Engaging);
			CombatDisplayTags.AddTag(AOStateTags::State_Combat_Combating);
			CombatDisplayTags.AddTag(AOStateTags::State_Combat_Preparation);
			CombatDisplayTags.AddTag(AOStateTags::State_Combat_Recovery);
			CombatDisplayTags.AddTag(AOStateTags::State_Combat_Block);
			CombatDisplayTags.AddTag(AOStateTags::State_Combat_Parried);
			CombatDisplayTags.AddTag(AOStateTags::State_Combat_Broken);
			bInitialized = true;
		}

		return CombatDisplayTags;
	}
}

UAOLocalTargetHealthBarObserverComponent::UAOLocalTargetHealthBarObserverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAOLocalTargetHealthBarObserverComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() == nullptr || GetWorld() == nullptr || GetWorld()->IsNetMode(NM_DedicatedServer))
	{
		return;
	}

	if (const APlayerController* PlayerController = Cast<APlayerController>(GetOwner()))
	{
		if (!PlayerController->IsLocalController())
		{
			return;
		}
	}
	else
	{
		return;
	}

	if (RefreshIntervalSeconds > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			RefreshObservedTargetsTimerHandle,
			this,
			&ThisClass::RefreshObservedTargets,
			RefreshIntervalSeconds,
			true);
	}
}

void UAOLocalTargetHealthBarObserverComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideAndForgetAllObservedTargets();

	if (GetWorld() != nullptr)
	{
		GetWorld()->GetTimerManager().ClearTimer(RefreshObservedTargetsTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UAOLocalTargetHealthBarObserverComponent::TrackObservedTargetFromCombatFeedback(const FAOCombatFeedbackViewData& FeedbackViewData)
{
	TrackTargetFromCombatFeedback(FeedbackViewData);
}

void UAOLocalTargetHealthBarObserverComponent::RefreshObservedTargets()
{
	AActor* LocalPlayerActor = GetLocalViewActor();
	if (LocalPlayerActor == nullptr)
	{
		HideAndForgetAllObservedTargets();
		return;
	}

	const float CurrentWorldTimeSeconds = GetWorld() != nullptr ? GetWorld()->GetTimeSeconds() : 0.0f;

	for (int32 Index = ObservedTargets.Num() - 1; Index >= 0; --Index)
	{
		if (!RefreshObservedTargetEntry(ObservedTargets[Index], CurrentWorldTimeSeconds, LocalPlayerActor))
		{
			ObservedTargets.RemoveAt(Index);
		}
	}
}

AActor* UAOLocalTargetHealthBarObserverComponent::GetLocalViewActor() const
{
	if (const APlayerController* PlayerController = Cast<APlayerController>(GetOwner()))
	{
		return PlayerController->GetPawn();
	}

	return nullptr;
}

bool UAOLocalTargetHealthBarObserverComponent::IsActorInCombatDisplayState(const AActor* Actor) const
{
	if (Actor == nullptr)
	{
		return false;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<AActor*>(Actor)))
	{
		FGameplayTagContainer OwnedTags;
		AbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);
		return OwnedTags.HasAny(AOLocalTargetHealthBarObserverComponentPrivate::GetCombatDisplayTags());
	}

	return false;
}

UAOTargetHealthBarComponent* UAOLocalTargetHealthBarObserverComponent::TrackTargetFromCombatFeedback(const FAOCombatFeedbackViewData& FeedbackViewData)
{
	if (!FeedbackViewData.bShouldEnqueueForHUD)
	{
		return nullptr;
	}

	AActor* TargetActor = FeedbackViewData.Target.Get();
	if (TargetActor == nullptr)
	{
		return nullptr;
	}

	UAOTargetHealthBarComponent* TargetHealthBarComponent = TargetActor->FindComponentByClass<UAOTargetHealthBarComponent>();
	if (TargetHealthBarComponent == nullptr || !TargetHealthBarComponent->IsWorldHealthBarEnabled())
	{
		return nullptr;
	}

	AActor* LocalPlayerActor = GetLocalViewActor();
	if (LocalPlayerActor == nullptr)
	{
		return nullptr;
	}

	const float CurrentWorldTimeSeconds = GetWorld() != nullptr ? GetWorld()->GetTimeSeconds() : 0.0f;
	for (FAOLocalObservedTargetHealthBarEntry& Entry : ObservedTargets)
	{
		if (Entry.TargetActor.Get() == TargetActor)
		{
			Entry.LastRelevantCombatTime = CurrentWorldTimeSeconds;
			Entry.TargetHealthBarComponent = TargetHealthBarComponent;
			RefreshObservedTargetEntry(Entry, CurrentWorldTimeSeconds, LocalPlayerActor);
			return TargetHealthBarComponent;
		}
	}

	FAOLocalObservedTargetHealthBarEntry& NewEntry = ObservedTargets.AddDefaulted_GetRef();
	NewEntry.TargetActor = TargetActor;
	NewEntry.TargetHealthBarComponent = TargetHealthBarComponent;
	NewEntry.LastRelevantCombatTime = CurrentWorldTimeSeconds;
	NewEntry.bWasRenderedLastRefresh = false;
	RefreshObservedTargetEntry(NewEntry, CurrentWorldTimeSeconds, LocalPlayerActor);
	return TargetHealthBarComponent;
}

bool UAOLocalTargetHealthBarObserverComponent::RefreshObservedTargetEntry(
	FAOLocalObservedTargetHealthBarEntry& Entry,
	float CurrentWorldTimeSeconds,
	const AActor* LocalPlayerActor) const
{
	AActor* TargetActor = Entry.TargetActor.Get();
	UAOTargetHealthBarComponent* TargetHealthBarComponent = Entry.TargetHealthBarComponent.Get();
	if (TargetActor == nullptr || TargetHealthBarComponent == nullptr || TargetHealthBarComponent->IsDead())
	{
		if (TargetHealthBarComponent != nullptr)
		{
			TargetHealthBarComponent->SetRequestedVisible(false);
		}
		return false;
	}

	const bool bHasCombatDisplayEligibility =
		IsActorInCombatDisplayState(LocalPlayerActor)
		|| IsActorInCombatDisplayState(TargetActor)
		|| ((CurrentWorldTimeSeconds - Entry.LastRelevantCombatTime) <= HideDelaySeconds);

	if (!bHasCombatDisplayEligibility)
	{
		TargetHealthBarComponent->SetRequestedVisible(false);
		return false;
	}

	const float DistanceSquared = FVector::DistSquared(LocalPlayerActor->GetActorLocation(), TargetActor->GetActorLocation());
	const float ShowDistanceSquared = FMath::Square(ShowDistance);
	const float HideDistanceSquared = FMath::Square(FMath::Max(HideDistance, ShowDistance));
	const bool bWithinDistance = Entry.bWasRenderedLastRefresh ? DistanceSquared <= HideDistanceSquared : DistanceSquared <= ShowDistanceSquared;

	TargetHealthBarComponent->SetRequestedVisible(bWithinDistance);
	Entry.bWasRenderedLastRefresh = bWithinDistance;
	return true;
}

void UAOLocalTargetHealthBarObserverComponent::HideAndForgetAllObservedTargets()
{
	for (FAOLocalObservedTargetHealthBarEntry& Entry : ObservedTargets)
	{
		if (UAOTargetHealthBarComponent* TargetHealthBarComponent = Entry.TargetHealthBarComponent.Get())
		{
			TargetHealthBarComponent->SetRequestedVisible(false);
		}
	}

	ObservedTargets.Reset();
}
