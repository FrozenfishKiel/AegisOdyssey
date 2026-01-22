// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "AOAssetManagerStartupJob.h"
#include "AOAssetManager.generated.h"
class UAOPawnData;
class UAOGameData;
/**
 * 
 */
UCLASS(Config = Game)
class AEGISODYSSEY_API UAOAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static UAOAssetManager& Get();

	template<typename AssetType>
	static AssetType* GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	template<typename AssetType>
	static TSubclassOf<AssetType> GetSubClass(const TSoftObjectPtr<AssetType>& AssetPointer , bool bKeepInMemory = true);
	static void DumpLoadedAssets();
	const UAOGameData& GetGameData();
	const UAOPawnData* GetPawnData();
protected:
	// Global game data asset to use.
	template<typename GameDataClass>
	const GameDataClass& GetOrLoadTypedGameData(const TSoftObjectPtr<GameDataClass>& DataPath)
	{
		
		//用TObjectPtr包装，防止被GC意外回收
		if (TObjectPtr<UPrimaryDataAsset> const * pResult = GameDataMap.Find(GameDataClass::StaticClass()))
		{
			return *CastChecked<GameDataClass>(*pResult);
		}

		return *CastChecked<const GameDataClass>(LoadGameDataOfClass(GameDataClass::StaticClass(),DataPath,GameDataClass::StaticClass()->GetFName()));
	}
	
	static UObject* SynchronousLoadAsset(const FSoftObjectPath& AssetPath);


	void AddLoadedAsset(const UObject* Asset);

	UPrimaryDataAsset* LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass, const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath, FPrimaryAssetType PrimaryAssetType);

	virtual void StartInitialLoading() override;
private:
	TArray<FAOAssetManagerStartupJob> StartupJobs;  //启动的资源加载任务列表
	void DoAllStartupJobs();
	void UpdateInitialGameContentLoadPercent(float GameContentPercent);
public:
	UPROPERTY(Config)
	TSoftObjectPtr<UAOGameData> AOGameData;

	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass> , TObjectPtr<UPrimaryDataAsset>> GameDataMap;

	UPROPERTY(Config)
	TSoftObjectPtr<UAOPawnData> AOPawnData;
	
private:
	UPROPERTY()
	TSet<TObjectPtr<const UObject>> LoadedAssets;  //这里保存的是不可修改的已加载的资源PrimaryAsset集合

	FCriticalSection LoadedAssetsCritical;  //递归互斥锁，允许某个线程多次访问，但只能一个线程访问，保护临界资源
	
};

//从资产软引用获取对应的资产（PrimaryAsset），Asset Getter不仅仅是加载和获取对应的资产，还会将加载后的资产存入到LoadedAsset中
template <typename AssetType>
AssetType* UAOAssetManager::GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	AssetType* LoadedAsset = nullptr;
	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();  //从传入的软引用Object中获取它的资源路径

	if (AssetPath.IsValid())
	{
		LoadedAsset = AssetPointer.Get();  //直接获取对应的资产（同步获取）
		if (!LoadedAsset)
		{
			LoadedAsset = Cast<AssetType>(SynchronousLoadAsset(AssetPath));  //同步加载为空尝试异步加载
			ensureAlwaysMsgf(LoadedAsset,TEXT("Failed to load asset [%s]") , *AssetPath.ToString());  //检查是否加载失败
		}

		if (LoadedAsset && bKeepInMemory)
		{
			Get().AddLoadedAsset(Cast<UObject>(LoadedAsset));
		}
	}
	return LoadedAsset;
}

template <typename AssetType>
TSubclassOf<AssetType> UAOAssetManager::GetSubClass(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	TSubclassOf<AssetType> LoadedSubClass;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		LoadedSubClass = AssetPointer.Get();
		if (!LoadedSubClass)
		{
			LoadedSubClass = Cast<UObject>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedSubClass,TEXT("Failed to load asset [%s]") , *AssetPath.ToString());  //检查是否加载失败
		}

		if (LoadedSubClass && bKeepInMemory)
		{
			Get().AddLoadedAsset(Cast<UObject>(LoadedSubClass));
		}
	}
		return LoadedSubClass;
}
