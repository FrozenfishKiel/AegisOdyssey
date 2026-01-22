// Fill out your copyright notice in the Description page of Project Settings.


#include "AOExperienceManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/AssetManager.h"
#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeaturesSubsystemSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOExperienceManagerComponent)


namespace AOConsoleVariables
{
	static float ExperienceLoadRandomDelayMin = 1.f;
	static FAutoConsoleVariableRef CVarExperienceLoadRandomDelayMin(
		TEXT("AO.chaos.ExperienceDelayLoad.MinSecs"),
		ExperienceLoadRandomDelayMin,
		TEXT("This value (in seconds) will be added as a delay of load completion of the experience (along with the random value lyra.chaos.ExperienceDelayLoad.RandomSecs)"),
		ECVF_Default);

	static float ExperienceLoadRandomDelayRange = 3.f;
	static FAutoConsoleVariableRef CVarExperienceLoadRandomDelayRange(
		TEXT("lyra.chaos.ExperienceDelayLoad.RandomSecs"),
		ExperienceLoadRandomDelayRange,
		TEXT("A random amount of time between 0 and this value (in seconds) will be added as a delay of load completion of the experience (along with the fixed value lyra.chaos.ExperienceDelayLoad.MinSecs)"),
		ECVF_Default);

	float GetExperienceLoadDelayDuration()
	{
		return FMath::Max(0.0f, ExperienceLoadRandomDelayMin + FMath::FRand() * ExperienceLoadRandomDelayRange);
	}
}


UAOExperienceManagerComponent::UAOExperienceManagerComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}


void UAOExperienceManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, CurrentExperience);
}
void UAOExperienceManagerComponent::SetCurrentExperience(FPrimaryAssetId ExperienceId)
{
	UAssetManager& AssetManager = UAssetManager::Get();
	FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(ExperienceId);
	TSubclassOf<UAOExperienceDefinition> AssetClass = Cast<UClass>(AssetPath.TryLoad());	check(AssetClass);
	const UAOExperienceDefinition* AODefinition = GetDefault<UAOExperienceDefinition>(AssetClass);

	check(AODefinition != nullptr);
	check(CurrentExperience == nullptr);
	CurrentExperience = AODefinition;
	StartExperienceLoad();
}

void UAOExperienceManagerComponent::SetCurrentExperience(const TSubclassOf<UAOExperienceDefinition> ExperienceRef)
{
	const UAOExperienceDefinition* AODefinition = GetDefault<UAOExperienceDefinition>(ExperienceRef);
	check(AODefinition != nullptr);
	check(CurrentExperience == nullptr);
	CurrentExperience = AODefinition;  //设置当前的游戏体验

	StartExperienceLoad();
}


void UAOExperienceManagerComponent::OnRep_CurrentExperience()
{
	StartExperienceLoad();
}

//收到并引用当前世界的ExperienceDefinition后交由该Experience管理器进行引用然后触发该函数
void UAOExperienceManagerComponent::StartExperienceLoad()
{
	check(CurrentExperience != nullptr);
	check(LoadState == EAOExperienceLoadState::Unloaded);  //确认当前处于未加载状态

	LoadState = EAOExperienceLoadState::Loading;  //进入加载状态

	UAssetManager& AssetManager = UAssetManager::Get();

	TSet<FPrimaryAssetId> BundleAssetList;
	TSet<FSoftObjectPath> RawAssetList;

	TArray<FName> BundlesToLoad;

	const ENetMode OwnerNetMode = GetOwner()->GetNetMode();  //获取当前Pawn的网络模式
	const bool bLoadClient = GIsEditor || (OwnerNetMode != NM_DedicatedServer); //如果当前处于服务器或客户端模式
	const bool bLoadServer = GIsEditor || (OwnerNetMode != NM_Client);

	if (bLoadClient)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateClient);
	}
	if (bLoadServer)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateServer);
	}
	

	TSharedPtr<FStreamableHandle> BundleLoadHandle = nullptr;
	if (BundleAssetList.Num() > 0)
	{
		BundleLoadHandle = AssetManager.ChangeBundleStateForPrimaryAssets(BundleAssetList.Array() , BundlesToLoad , {} , false , FStreamableDelegate() , FStreamableManager::AsyncLoadHighPriority);
	}

	TSharedPtr<FStreamableHandle> RawLoadHandle = nullptr;

	if (RawAssetList.Num() > 0)
	{
		RawLoadHandle = AssetManager.LoadAssetList(RawAssetList.Array(), FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority, TEXT("StartExperienceLoad()"));
	}

	// If both async loads are running, combine them
	TSharedPtr<FStreamableHandle> Handle = nullptr;
	if (BundleLoadHandle.IsValid() && RawLoadHandle.IsValid())
	{
		Handle = AssetManager.GetStreamableManager().CreateCombinedHandle({ BundleLoadHandle, RawLoadHandle });
	}
	else
	{
		Handle = BundleLoadHandle.IsValid() ? BundleLoadHandle : RawLoadHandle;
	}
	
	FStreamableDelegate OnAssetsLoadedDelegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnGameFeaturePluginLoaded);
	if (!Handle.IsValid() || Handle->HasLoadCompleted())
	{
		// Assets were already loaded, call the delegate now
		FStreamableHandle::ExecuteDelegate(OnAssetsLoadedDelegate);
	}
	else
	{
		Handle->BindCompleteDelegate(OnAssetsLoadedDelegate);

		Handle->BindCancelDelegate(FStreamableDelegate::CreateLambda([OnAssetsLoadedDelegate]()
			{
				OnAssetsLoadedDelegate.ExecuteIfBound();
			}));
	}
}

bool UAOExperienceManagerComponent::IsExperienceLoaded() const
{
	return (LoadState == EAOExperienceLoadState::Loaded && CurrentExperience != nullptr);
}

const UAOExperienceDefinition* UAOExperienceManagerComponent::GetCurrentExperienceCheck() const
{
	check(LoadState == EAOExperienceLoadState::Loaded);
	check(CurrentExperience != nullptr);
	return CurrentExperience;
}

void UAOExperienceManagerComponent::CallRegister_OnExperienceLoaded(
	FOnExperienceLoaded::FDelegate&& OnExperienceLoadedDelegate)
{
	if (IsExperienceLoaded())
	{
		OnExperienceLoadedDelegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded.Add(MoveTemp(OnExperienceLoadedDelegate));  //如果调用此函数的时候走到了这一步，那么就会保存通知变量，等待下一次一起通知
	}
}

void UAOExperienceManagerComponent::OnGameFeaturePluginLoaded()
{
	check(LoadState == EAOExperienceLoadState::Loading);
	check(CurrentExperience != nullptr);
	
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

	CollectGameFeaturePluginURLs(CurrentExperience, CurrentExperience->GameFeatureNames);


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

void UAOExperienceManagerComponent::OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result)
{
	NumGameFeaturePluginsLoading--;

	if (NumGameFeaturePluginsLoading <= 0)
	{
		OnGameFeatureLoadedReady();
	}
}


void UAOExperienceManagerComponent::OnGameFeatureLoadedReady()
{
	check(LoadState != EAOExperienceLoadState::Loaded);

	

	// 混沌变量测试：测试Experience在实际游玩的各种情况（例如网络延迟，配置卡顿等）下的Experience的加载问题
	// 这种方法偏向异步
	if (LoadState != EAOExperienceLoadState::LoadingChaosTestingDelay)
	{
		const float DelaySecs = AOConsoleVariables::GetExperienceLoadDelayDuration();

		if (DelaySecs > 0)
		{
			FTimerHandle TimerHandle;

			LoadState = EAOExperienceLoadState::LoadingChaosTestingDelay;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle , this , &ThisClass::OnGameFeatureLoadedReady,DelaySecs,false);

			return;
		}
	}


	
	
	LoadState = EAOExperienceLoadState::ExecutingActions;

		
	
	LoadState = EAOExperienceLoadState::Loaded;  //标记为已加载
	
	
	OnExperienceLoaded.Broadcast(CurrentExperience);  //将先前所有注册的都广播了
	OnExperienceLoaded.Clear();  //然后清除

	
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

	ActivationListOfActions(CurrentExperience->Actions);

		
	OnExperienceLoaded.Broadcast(CurrentExperience);  //将先前所有注册的都广播了
	OnExperienceLoaded.Clear();  //然后清除
}

