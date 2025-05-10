// Fill out your copyright notice in the Description page of Project Settings.


#include "GF_AddInputMapping.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GF_AddInputMapping)

void UGF_AddInputMapping::OnGameFeatureRegistering()
{
	Super::OnGameFeatureRegistering();

	RegisterInputMappingContexts();
}

void UGF_AddInputMapping::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	Super::OnGameFeatureActivating(Context);
}

void UGF_AddInputMapping::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
}

void UGF_AddInputMapping::OnGameFeatureUnregistering()
{
	Super::OnGameFeatureUnregistering();
}

void UGF_AddInputMapping::RegisterInputMappingContexts()
{
	//注册World委托：当GameInstance开始的时候，委托回调
	RegisterInputContextMappingsForGameInstanceHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this,&UGF_AddInputMapping::RegisterInputContextMappingsForGameInstance);

	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();  //获取所有的世界上下文
	for (TIndirectArray<FWorldContext>::TConstIterator WorldContextIterator = WorldContexts.CreateConstIterator(); WorldContextIterator; ++WorldContextIterator)
	{
		RegisterInputContextMappingsForGameInstance(WorldContextIterator->OwningGameInstance);
	}
}

void UGF_AddInputMapping::RegisterInputContextMappingsForGameInstance(UGameInstance* GameInstance)
{
	if (GameInstance != nullptr && !GameInstance->OnLocalPlayerAddedEvent.IsBoundToObject(this))
	{
		GameInstance->OnLocalPlayerAddedEvent.AddUObject(this,&UGF_AddInputMapping::RegisterInputMappingContextsForLocalPlayer);
		GameInstance->OnLocalPlayerAddedEvent.AddUObject(this,&UGF_AddInputMapping::UnregisterInputMappingContextsForLocalPlayer);
	}
}

void UGF_AddInputMapping::RegisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer)
{
	
}

void UGF_AddInputMapping::UnregisterInputMappingContexts()
{
	
}

void UGF_AddInputMapping::UnregisterInputContextMappingsForGameInstance(UGameInstance* GameInstance)
{
	
}

void UGF_AddInputMapping::UnregisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer)
{
	
}