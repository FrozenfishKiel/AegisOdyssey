// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction_WordActionBase.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySet.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GF_AddAbilities.generated.h"

/**
 * 
 */
USTRUCT()
struct FGameFeatureAbilitiesEntry
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere,Category = "Attributes")
	TSoftClassPtr<AActor> ActorClass;

	

	UPROPERTY(EditAnywhere,Category = "Attributes",meta=(AssetBundles="Client,Server"))
	TArray<TSoftObjectPtr<const UAOAbilitySet>> GrantedAbilitySets;
};
UCLASS()
class AEGISODYSSEY_API UGF_AddAbilities : public UGameFeatureAction_WordActionBase
{
	GENERATED_BODY()
public:
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

	UPROPERTY(EditAnywhere,Category = "Abilities",meta=(TitleProperty="ActorClass", ShowOnlyInnerProperties))
	TArray<FGameFeatureAbilitiesEntry> AbilitiesList;

private:
	struct FActorExtensions
	{
		TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;
		TArray<UAttributeSet*> Attributes;
		//TArray<FLyraAbilitySet_GrantedHandles> AbilitySetHandles;
	};
	struct FPerContextData
	{
		TMap<AActor*, FActorExtensions> ActiveExtensions;
		TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequestHandles;
	};
	TMap<FGameFeatureStateChangeContext,FPerContextData> ContextData;

	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	void AddActorAbilities(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData);
	void Reset(FPerContextData& ActiveData);
	void HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex, FGameFeatureStateChangeContext ChangeContext);

	template<class ComponentType>
	ComponentType* FindOrAddComponentForActor(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData)
	{
		return Cast<ComponentType>(FindOrAddComponentForActor(ComponentType::StaticClass(), Actor, AbilitiesEntry, ActiveData));
	}
};
