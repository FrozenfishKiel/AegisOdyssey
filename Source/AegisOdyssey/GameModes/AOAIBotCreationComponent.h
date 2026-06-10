// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "AegisOdyssey/Player/AAOAIPlayerBotController.h"
#include "AOAIBotCreationComponent.generated.h"

class AAIController;
class UAOExperienceDefinition;

/**
 * AOAIBotCreationComponent
 *
 *	用于创建和管理AI Bot的组件
 *	支持创建永久AI（可重生）和临时AI（死亡销毁）
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOAIBotCreationComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	UAOAIBotCreationComponent(const FObjectInitializer& ObjectInitializer);

	// 获取已生成的Bot列表
	const TArray<AAIController*>& GetSpawnedBotList() const { return SpawnedBotList; }

	// 手动创建一个Bot（使用默认配置）
	UFUNCTION(BlueprintCallable)
	void SpawnOneBot();

	// 手动创建一个指定类型的Bot
	UFUNCTION(BlueprintCallable)
	AAIController* SpawnBotWithType(EAIBotType BotType);

	// 手动移除一个Bot
	UFUNCTION(BlueprintCallable)
	void RemoveOneBot();

	// 设置Bot数量
	UFUNCTION(BlueprintCallable)
	void SetNumBotsToCreate(int32 InNumBots) { NumBotsToCreate = InNumBots; }

	// 获取默认AI类型
	EAIBotType GetDefaultBotType() const { return DefaultBotType; }

	// 设置默认AI类型
	void SetDefaultBotType(EAIBotType InType) { DefaultBotType = InType; }

protected:
	virtual void BeginPlay() override;

	// 当Experience加载完成时调用
	void OnExperienceLoaded(const UAOExperienceDefinition* Experience);

	// 服务器端创建Bots
	UFUNCTION(Server, Reliable)
	void ServerCreateBots();
	void ServerCreateBots_Implementation();

	// 创建Bot名称
	FString CreateBotName(int32 PlayerIndex);

protected:
	// Bot使用的AIController类
	UPROPERTY(EditAnywhere, Category = "Bot Creation")
	TSubclassOf<AAIController> BotControllerClass;

	// 要创建的Bot数量
	UPROPERTY(EditAnywhere, Category = "Bot Creation")
	int32 NumBotsToCreate = 0;

	// 默认AI类型（首次生成时使用）
	UPROPERTY(EditAnywhere, Category = "Bot Creation")
	EAIBotType DefaultBotType = EAIBotType::Temporary;

	// 随机Bot名称列表
	UPROPERTY(EditAnywhere, Category = "Bot Creation")
	TArray<FString> RandomBotNames;

	// 已生成的Bot列表
	UPROPERTY()
	TArray<TObjectPtr<AAIController>> SpawnedBotList;

	// 剩余的Bot名称（用于生成唯一名称）
	UPROPERTY()
	TArray<FString> RemainingBotNames;
};
