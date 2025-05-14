// Fill out your copyright notice in the Description page of Project Settings.


#include "AOGameMode.h"
#include "AOGameFeatureDefinition.h"
#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOGameMode)
void AAOGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	GetWorld()->GetTimerManager().SetTimerForNextTick(this,&ThisClass::HandleGameInitialize);  //在下一帧执行
}

APawn* AAOGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;	// Never save the default player pawns into a map.
	SpawnInfo.bDeferConstruction = true;

	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		if (APawn* Spawned = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform,SpawnInfo))
		{
			if (UAOExtPawnComponent* PawnExtComp = UAOExtPawnComponent::FindAOExtPawnComponent(Spawned))
			{
				PawnExtComp->CheckDefaultInitialization();
			}

			Spawned->FinishSpawning(SpawnTransform);

			return Spawned;
		}
	}

	return nullptr;
}

void AAOGameMode::HandleGameInitialize()
{
	check(GameFeatureDefinition);
	OnGameFeaturePluginLoaded();
}

void AAOGameMode::OnGameFeaturePluginLoaded()
{
	GameFeaturePluginURLs.Reset();

	auto CollectGameFeaturePluginURLs = [This=this](const UPrimaryDataAsset* Context, const TArray<FString>& FeaturePluginList)
	{
		for (const FString& PluginName : FeaturePluginList)
		{
			FString PluginURL;
			//获取的URL会储存在PluginURL上
			if (UGameFeaturesSubsystem::Get().GetPluginURLByName(PluginName, /*out*/ PluginURL))
			{
				This->GameFeaturePluginURLs.AddUnique(PluginURL);
			}
			else
			{
				ensureMsgf(false, TEXT("OnExperienceLoadComplete failed to find plugin URL from PluginName %s for experience %s - fix data, ignoring for this run"), *PluginName, *Context->GetPrimaryAssetId().ToString());
			}
		}
	};

	CollectGameFeaturePluginURLs(GameFeatureDefinition, GameFeatureDefinition->GameFeatureNames);

	NumGameFeaturePluginsLoading = GameFeaturePluginURLs.Num();
	if (NumGameFeaturePluginsLoading > 0)
	{
		for (const FString& PluginURL : GameFeaturePluginURLs)
		{
			//加载并激活对应的GF Action
			UGameFeaturesSubsystem::Get().LoadAndActivateGameFeaturePlugin(PluginURL,FGameFeaturePluginLoadComplete::CreateUObject(this, &ThisClass::OnGameFeaturePluginLoadComplete));
		}
	}
	else
	{
		OnGameFeatureLoadedReady();
	}
}

void AAOGameMode::OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result)
{
	NumGameFeaturePluginsLoading--;

	if (NumGameFeaturePluginsLoading <= 0)
	{
		OnGameFeatureLoadedReady();
	}
}

void AAOGameMode::OnGameFeatureLoadedReady()
{
	FGameFeatureActivatingContext Context;
	// Only apply to our specific world context if set
	//只选取与我们世界有关的上下文
	const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
	if (ExistingWorldContext)
	{
		Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
	}
	
	auto ActivationListOfActions = [&Context] (const TArray<UGameFeatureAction*>& InActions)
	{
		for (UGameFeatureAction* Action : InActions)
		{
			if (Action != nullptr)
			{
				Action->OnGameFeatureRegistering();
				Action->OnGameFeatureLoading();
				Action->OnGameFeatureActivating(Context);
			}
		}
	};

	ActivationListOfActions(GameFeatureDefinition->Actions);
}
