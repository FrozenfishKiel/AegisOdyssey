// Fill out your copyright notice in the Description page of Project Settings.


#include "GF_AddAbilities.h"

#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/Player/AOPlayerState.h"
#include "UObject/FastReferenceCollector.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GF_AddAbilities)

void UGF_AddAbilities::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{

	FPerContextData& ActivateData = ContextData.FindOrAdd(Context);  //查找或添加当前GF激活上下文
	if (!ensureAlways(ActivateData.ComponentRequestHandles.IsEmpty()))
	{
		Reset(ActivateData);
	}
	
	Super::OnGameFeatureActivating(Context);
	
}

void UGF_AddAbilities::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
	FPerContextData* ActivateData = ContextData.Find(Context);
	if (ensure(ActivateData))
	{
		Reset(*ActivateData);
	}
}

void UGF_AddAbilities::AddToWorld(const FWorldContext& WorldContext,
	const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActivateData = ContextData.FindOrAdd(ChangeContext);

	if ((GameInstance!=nullptr) && (World!=nullptr) && World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* ComponentMan = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			int32 EntryIndex = 0;
			for (const FGameFeatureAbilitiesEntry& Entry : AbilitiesList)
			{
				if (!Entry.ActorClass.IsNull())
				{
					UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilitiesDelegate = UGameFrameworkComponentManager::
					FExtensionHandlerDelegate::CreateUObject(this,&UGF_AddAbilities::HandleActorExtension,EntryIndex,ChangeContext);
					TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentMan->AddExtensionHandler(
						Entry.ActorClass,AddAbilitiesDelegate);

					ActivateData.ComponentRequestHandles.Add(ExtensionRequestHandle);
					EntryIndex++;
				}
			}
		}
	}
}

void UGF_AddAbilities::Reset(FPerContextData& ActiveData)
{
	while (!ActiveData.ActiveExtensions.IsEmpty())
	{
		auto ExtensionIt = ActiveData.ActiveExtensions.CreateIterator();
	}
	ActiveData.ComponentRequestHandles.Empty();
}

void UGF_AddAbilities::HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex,
                                            FGameFeatureStateChangeContext ChangeContext)
{
	FPerContextData* ActiveData = ContextData.Find(ChangeContext);
	if (AbilitiesList.IsValidIndex(EntryIndex) && ActiveData)
	{
		const FGameFeatureAbilitiesEntry& Entry = AbilitiesList[EntryIndex];
		if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded || EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
		{
			
		}
		else if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded || EventName == AAOPlayerState::NAME_AOAbilityReady)
		{
			AddActorAbilities(Actor,Entry,*ActiveData);
		}
	}
}

void UGF_AddAbilities::AddActorAbilities(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry,
	FPerContextData& ActiveData)
{
	check(Actor);
	if (!Actor->HasAuthority()) return;
	if (ActiveData.ActiveExtensions.Find(Actor) != nullptr) return;

	if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
	{
		FActorExtensions AddExtensions;

		AddExtensions.Attributes.Reserve(AbilitiesEntry.GrantedAbilitySets.Num());
	}
}
