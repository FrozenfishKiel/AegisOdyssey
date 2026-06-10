// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction_WordActionBase.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySet.h"
#include "AegisOdyssey/AbilitySystem/Abilities/AOGameplayAbility.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GF_AddAbilities.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FAOAbilityGrant
{
	GENERATED_BODY()

	// Type of ability to grant
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AssetBundles="Client,Server"))
	TSoftClassPtr<UGameplayAbility> AbilityType;

	// Input action to bind the ability to, if any (can be left unset)
	// 	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	// 	TSoftObjectPtr<UInputAction> InputAction;
};
USTRUCT(BlueprintType)
struct FAOAttributeSetGrant
{
	GENERATED_BODY()

	// Ability set to grant
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AssetBundles="Client,Server"))
	TSoftClassPtr<UAttributeSet> AttributeSetType;
	
};
//该结构体保存的是可赋予的角色技能表，和属性表
USTRUCT()
struct FGameFeatureAbilitiesEntry
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere,Category = "Attributes")
	TSoftClassPtr<AActor> ActorClass;

	// List of abilities to grant to actors of the specified class
	UPROPERTY(EditAnywhere, Category="Abilities")
	TArray<FAOAbilityGrant> GrantedAbilities;  //赋予角色的技能表

	UPROPERTY(EditAnywhere,Category = "Attributes")
	TArray<FAOAttributeSetGrant> AttributeClass;
	
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
	
	struct FActorExtensions
	{
		TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;  //保存角色已经激活的技能
		TArray<UAttributeSet*> Attributes; //保存角色已经激活的属性
		TArray<FAOAbilitySet_GrantedHandles> AbilitySetHandles;
	};

private:

	struct FPerContextData
	{
		//角色和他已经拥有并激活的技能和属性
		TMap<AActor*, FActorExtensions> ActiveExtensions;
		TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequestHandles;
	};
	TMap<FGameFeatureStateChangeContext,FPerContextData> ContextData;

	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;
	void AddActorAbilities(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData);
	void RemoveActorAbilities(AActor* Actor, FPerContextData& ActiveData);
	void Reset(FPerContextData& ActiveData);
	void HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex, FGameFeatureStateChangeContext ChangeContext);

	template<class ComponentType>
	ComponentType* FindOrAddComponentForActor(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData)
	{
		return Cast<ComponentType>(FindOrAddComponentForActor(ComponentType::StaticClass(), Actor, AbilitiesEntry, ActiveData));
	}
};
