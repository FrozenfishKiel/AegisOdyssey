#pragma once

#include "Engine/DataAsset.h"
#include "AOGameData.generated.h"

class UGameplayEffect;
// AegisOdyssey的游戏主要资产文件
UCLASS(BlueprintType , Const , Meta = (DisplayName = "AO GameData" , ShortTooltip = "Data Asset Containing Global Game Data."))
class AEGISODYSSEY_API UAOGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UAOGameData();

	static const UAOGameData& Get();  //静态获取全局唯一的方法（饿汉式）

	// 重写GetPrimaryAssetId方法，定义资产的PrimaryAssetType
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("AOGameData", GetFName());
	}

public:
	UPROPERTY(EditDefaultsOnly, Category = "AO GameData" , meta = (DisplayName = "Damage Gameplay Effect (Set By Caller)"))
	TSoftClassPtr<UGameplayEffect> DamageGameplayEffect_SetByCaller;  //角色通过标签造成的伤害效果，从资源中加载

	UPROPERTY(EditDefaultsOnly , Category = "AO GameData" , meta = (EditCondition = "HealGameplayEffect (Set By Caller)"))
	TSoftClassPtr<UGameplayEffect> HealGameplayEffect_SetByCaller; //角色通过标签回复血量的效果，从资源中加载

	UPROPERTY(EditDefaultsOnly , Category = "AO GameData" , meta = (EditCondition = "DynamicTagGameplayEffect (Set By Caller)"))
	TSoftClassPtr<UGameplayEffect> DynamicTagGameplayEffect_SetByCaller;  //通过效果来动态地给角色赋予标签的效果
};