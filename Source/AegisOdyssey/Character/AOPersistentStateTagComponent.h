// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Components/PawnComponent.h"
#include "AOPersistentStateTagComponent.generated.h"

class UAbilitySystemComponent;

USTRUCT()
struct FAOPersistentStateTagEntry
{
	GENERATED_BODY()

	FAOPersistentStateTagEntry() = default;

	FAOPersistentStateTagEntry(const FGameplayTagContainer& InGrantedTags, const FActiveGameplayEffectHandle& InEffectHandle)
		: GrantedTags(InGrantedTags)
		, EffectHandle(InEffectHandle)
	{
	}

	FGameplayTagContainer GrantedTags;
	FActiveGameplayEffectHandle EffectHandle;
};

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOPersistentStateTagComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UAOPersistentStateTagComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintPure, Category = "AO|PersistentStateTags")
	static UAOPersistentStateTagComponent* FindPersistentStateTagComponent(const AActor* Actor)
	{
		return Actor ? Actor->FindComponentByClass<UAOPersistentStateTagComponent>() : nullptr;
	}

	UFUNCTION(BlueprintCallable, Category = "AO|PersistentStateTags")
	bool EnsureTagsGranted(FName SourceId, const FGameplayTagContainer& Tags);

	UFUNCTION(BlueprintCallable, Category = "AO|PersistentStateTags")
	bool ClearTagsBySource(FName SourceId);

	UFUNCTION(BlueprintCallable, Category = "AO|PersistentStateTags")
	bool HasSource(FName SourceId) const;

	UFUNCTION(BlueprintCallable, Category = "AO|PersistentStateTags")
	bool HasActiveTagsForSource(FName SourceId) const;

	UFUNCTION(BlueprintCallable, Category = "AO|PersistentStateTags")
	void ClearAllSources();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool RemoveTrackedSource(UAbilitySystemComponent* AbilitySystemComponent, FName SourceId);
	UAbilitySystemComponent* ResolveAbilitySystemComponent(AActor* TargetActor) const;

	TMap<FName, FAOPersistentStateTagEntry> ActiveTagEntries;
};
