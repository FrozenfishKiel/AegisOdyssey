// Fill out your copyright notice in the Description page of Project Settings.

#include "AOPersistentStateTagComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "AegisOdyssey/AbilitySystem/AOEffects/GE_PersistentStateTags.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOPersistentStateTagComponent)

UAOPersistentStateTagComponent::UAOPersistentStateTagComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool UAOPersistentStateTagComponent::EnsureTagsGranted(FName SourceId, const FGameplayTagContainer& Tags)
{
	if (SourceId.IsNone() || Tags.IsEmpty())
	{
		return false;
	}

	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(GetOwner());
	if (AbilitySystemComponent == nullptr)
	{
		return false;
	}

	if (FAOPersistentStateTagEntry* ExistingEntry = ActiveTagEntries.Find(SourceId))
	{
		const bool bHasSameTags = ExistingEntry->GrantedTags == Tags;
		const bool bHandleIsActive = ExistingEntry->EffectHandle.IsValid()
			&& AbilitySystemComponent->GetActiveGameplayEffect(ExistingEntry->EffectHandle) != nullptr;

		if (bHasSameTags && bHandleIsActive)
		{
			return true;
		}

		RemoveTrackedSource(AbilitySystemComponent, SourceId);
	}

	FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		UGE_PersistentStateTags::StaticClass(),
		1.0f,
		EffectContextHandle);

	if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
	{
		return false;
	}

	SpecHandle.Data->DynamicGrantedTags.AppendTags(Tags);

	const FActiveGameplayEffectHandle AppliedHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	if (!AppliedHandle.IsValid())
	{
		return false;
	}

	ActiveTagEntries.Add(SourceId, FAOPersistentStateTagEntry(Tags, AppliedHandle));
	return true;
}

bool UAOPersistentStateTagComponent::ClearTagsBySource(FName SourceId)
{
	if (SourceId.IsNone())
	{
		return false;
	}

	if (!ActiveTagEntries.Contains(SourceId))
	{
		return false;
	}

	return RemoveTrackedSource(ResolveAbilitySystemComponent(GetOwner()), SourceId);
}

bool UAOPersistentStateTagComponent::HasSource(FName SourceId) const
{
	return !SourceId.IsNone() && ActiveTagEntries.Contains(SourceId);
}

bool UAOPersistentStateTagComponent::HasActiveTagsForSource(FName SourceId) const
{
	if (SourceId.IsNone())
	{
		return false;
	}

	const FAOPersistentStateTagEntry* Entry = ActiveTagEntries.Find(SourceId);
	if (Entry == nullptr)
	{
		return false;
	}

	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(GetOwner());
	return AbilitySystemComponent != nullptr
		&& Entry->EffectHandle.IsValid()
		&& AbilitySystemComponent->GetActiveGameplayEffect(Entry->EffectHandle) != nullptr;
}

void UAOPersistentStateTagComponent::ClearAllSources()
{
	if (ActiveTagEntries.IsEmpty())
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponent(GetOwner());
	TArray<FName> SourceIds;
	ActiveTagEntries.GenerateKeyArray(SourceIds);

	for (const FName SourceId : SourceIds)
	{
		RemoveTrackedSource(AbilitySystemComponent, SourceId);
	}
}

void UAOPersistentStateTagComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearAllSources();
	Super::EndPlay(EndPlayReason);
}

bool UAOPersistentStateTagComponent::RemoveTrackedSource(UAbilitySystemComponent* AbilitySystemComponent, FName SourceId)
{
	FAOPersistentStateTagEntry* Entry = ActiveTagEntries.Find(SourceId);
	if (Entry == nullptr)
	{
		return false;
	}

	bool bRemoved = true;
	if (AbilitySystemComponent != nullptr && Entry->EffectHandle.IsValid())
	{
		bRemoved = AbilitySystemComponent->RemoveActiveGameplayEffect(Entry->EffectHandle, 1);
	}

	ActiveTagEntries.Remove(SourceId);
	return bRemoved;
}

UAbilitySystemComponent* UAOPersistentStateTagComponent::ResolveAbilitySystemComponent(AActor* TargetActor) const
{
	if (TargetActor == nullptr)
	{
		return nullptr;
	}

	if (APlayerState* PlayerState = Cast<APlayerState>(TargetActor))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerState))
		{
			return AbilitySystemComponent;
		}
	}

	if (APawn* Pawn = Cast<APawn>(TargetActor))
	{
		if (APlayerState* PlayerState = Pawn->GetPlayerState())
		{
			if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerState))
			{
				return AbilitySystemComponent;
			}
		}
	}

	if (AController* Controller = Cast<AController>(TargetActor))
	{
		if (APlayerState* PlayerState = Controller->PlayerState)
		{
			if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerState))
			{
				return AbilitySystemComponent;
			}
		}
	}

	if (const UAOExtPawnComponent* PawnExtComponent = UAOExtPawnComponent::FindAOExtPawnComponent(TargetActor))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = PawnExtComponent->GetAbilitySystemComponent())
		{
			return AbilitySystemComponent;
		}
	}

	return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
}
