// Fill out your copyright notice in the Description page of Project Settings.


#include "AOAssetManager.h"
#include "AOAssetManagerStartupJob.h"
#include "AOGameData.h"
#include "AegisOdyssey/Character/AOPawnData.h"
#include "AegisOdyssey/AOLogChannels.h"
static FAutoConsoleCommand CVarDumpLoadedAssets(
	TEXT("AegisOdyssey.DumpLoadedAssets"),                                                                                                                    
	TEXT("Shows all assets that were loaded via the asset manager and are currently in memory."),
	FConsoleCommandDelegate::CreateStatic(UAOAssetManager::DumpLoadedAssets)
);

#define STARTUP_JOB_WEIGHTED(JobFunc,JobWeight) StartupJobs.Add(FAOAssetManagerStartupJob(#JobFunc,[this](const FAOAssetManagerStartupJob& StartupJob,TSharedPtr<FStreamableHandle>& LoadHandkle){JobFunc;},JobWeight));
#define STARTUP_JOB(JobFunc) STARTUP_JOB_WEIGHTED(JobFunc,1.f);

// 资产管理开始初始化（由引擎启动）
void UAOAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	STARTUP_JOB_WEIGHTED(GetGameData(), 25.f); //加载游戏数据，并分配权重固定为25；

	DoAllStartupJobs();  //一次性执行任务队列
}

void UAOAssetManager::DoAllStartupJobs()
{
	SCOPED_BOOT_TIMING("ULyraAssetManager::DoAllStartupJobs");
	const double AllStartupJobsStartTime = FPlatformTime::Seconds();

	if (IsRunningDedicatedServer())
	{
		for (const FAOAssetManagerStartupJob& StartupJob : StartupJobs)
		{
			StartupJob.DoJob();
		}
	}
	//通常是客户端需要加载资源，服务器直接加载即可
	else
	{
		if (StartupJobs.Num() > 0)
		{
			float TotalJobValue = 0.0f;

			for (const FAOAssetManagerStartupJob& StartupJob : StartupJobs)
			{
				TotalJobValue += StartupJob.JobWeight;  //累加权重值，这一步是为了计算所有的资源加载总共需要预分配多少时间，这个时间会在下面用来计算转换成实际看到的进度条数据
			}
			float AccumlatedJobValue = 0.f;
			for (FAOAssetManagerStartupJob& StartupJob : StartupJobs)
			{
				const float JobValue = StartupJob.JobWeight;
				StartupJob.SubStepProgressDelegate.BindLambda([This = this,AccumlatedJobValue,JobValue,TotalJobValue](float NewProgress)
				{
					const float SubstepAdjustment = FMath::Clamp(NewProgress,0.f,1.f) * JobValue;
					const float OverallPercentWithSubstep = (AccumlatedJobValue - SubstepAdjustment) / TotalJobValue;

					This->UpdateInitialGameContentLoadPercent(OverallPercentWithSubstep);
				});

				StartupJob.DoJob();

				StartupJob.SubStepProgressDelegate.Unbind();

				AccumlatedJobValue += JobValue;  //累加权重值

				UpdateInitialGameContentLoadPercent(AccumlatedJobValue / TotalJobValue);
			}
		}
		else
		{
			UpdateInitialGameContentLoadPercent(1.f);
		}
	}
}
//更新的资源加载进度（进度条会需要这个）
void UAOAssetManager::UpdateInitialGameContentLoadPercent(float GameContentPercent)
{
	
}

UAOAssetManager& UAOAssetManager::Get()
{
	check(GEngine);

	if (UAOAssetManager* Singleton = Cast<UAOAssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}

	UE_LOG(LogAegisOdyssey, Fatal, TEXT("Invalid AssetManagerClassName in DefaultEngine.ini.  It must be set to AOAssetManager!"));

	return *NewObject<UAOAssetManager>();
}



UObject* UAOAssetManager::SynchronousLoadAsset(const FSoftObjectPath& AssetPath)
{
	if (AssetPath.IsValid())
	{
		if (UAssetManager::IsInitialized())
		{
			return UAssetManager::GetStreamableManager().LoadSynchronous(AssetPath,false);  //调用流式加载管理器去异步加载资源
			
		}
		return AssetPath.TryLoad();  //如果资产管理器未初始化直接同步加载
	}
	return nullptr;
}

void UAOAssetManager::DumpLoadedAssets()
{
	//打印已经加载的所有资源，通知已加载的资源池
	UE_LOG(LogAegisOdyssey, Log, TEXT("========== Start Dumping Loaded Assets =========="));

	for (const UObject* LoadedAsset : Get().LoadedAssets)
	{
		UE_LOG(LogAegisOdyssey, Log, TEXT("  %s"), *GetNameSafe(LoadedAsset));
	}

	UE_LOG(LogAegisOdyssey, Log, TEXT("... %d assets in loaded pool"), Get().LoadedAssets.Num());
	UE_LOG(LogAegisOdyssey, Log, TEXT("========== Finish Dumping Loaded Assets =========="));
}

//这个是动态加载的手段，它一般不会在游戏开始前加载，而是在游戏过程中动态地进行加载，因此也需要进行线程保护
void UAOAssetManager::AddLoadedAsset(const UObject* Asset)
{
	if (ensureAlways(Asset))
	{
		FScopeLock LoadedAssetsLock(&LoadedAssetsCritical);
		LoadedAssets.Add(Asset);  //规定单个线程添加资源（保护临界资源）
	}
}

UPrimaryDataAsset* UAOAssetManager::LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass,
	const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath, FPrimaryAssetType PrimaryAssetType)
{
	UPrimaryDataAsset* Asset = nullptr;

	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Loading GameData Object."),STAT_GameData,STATGROUP_LoadTime);
	if (!DataClassPath.IsNull())
	{
#if WITH_EDITOR
		FScopedSlowTask SlowTask(0, FText::Format(NSLOCTEXT("LyraEditor", "BeginLoadingGameDataTask", "Loading GameData {0}"), FText::FromName(DataClass->GetFName())));
		const bool bShowCancelButton = false;
		const bool bAllowInPIE = true;
		SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);
#endif
	}
	UE_LOG(LogAegisOdyssey, Log, TEXT("Loading GameData: %s ..."), *DataClassPath.ToString());
	SCOPE_LOG_TIME_IN_SECONDS(TEXT("    ... GameData loaded!"), nullptr);

	if (GIsEditor)
	{
		Asset = DataClassPath.LoadSynchronous();
		LoadPrimaryAssetsWithType(PrimaryAssetType);
	}
	else
	{
		TSharedPtr<FStreamableHandle> Handle = LoadPrimaryAssetsWithType(PrimaryAssetType);
		if (Handle.IsValid())
		{
			Handle->WaitUntilComplete(0.0f, false);

			// This should always work
			Asset = Cast<UPrimaryDataAsset>(Handle->GetLoadedAsset());
		}
	}

	if (Asset != nullptr)
	{
		GameDataMap.Add(DataClass, Asset);
		AddLoadedAsset(Asset);
	}

	return Asset;
}

const UAOGameData& UAOAssetManager::GetGameData()
{
	return GetOrLoadTypedGameData<UAOGameData>(AOGameData);
}

const UAOPawnData* UAOAssetManager::GetPawnData()
{
	return GetAsset(AOPawnData);
}
