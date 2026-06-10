// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOExperienceManagerComponent.h"
#include "AegisOdyssey/GameModes/AOAIBotCreationComponent.h"
#include "ModularGameState.h"
#include "AegisOdyssey/Player/AOPlayerSpawningManagerComponent.h"
#include "AOGameState.generated.h"

/**
 * AOGameState
 *
 *	游戏状态类，管理全局游戏组件
 */
UCLASS()
class AEGISODYSSEY_API AAOGameState : public AModularGameStateBase
{
	GENERATED_BODY()
public:
	AAOGameState();

	// 获取AI Bot创建组件
	UFUNCTION(BlueprintCallable, Category = "AO|GameState")
	UAOAIBotCreationComponent* GetAIBotCreationComponent() const { return AIBotCreationComponent; }

private:
	UPROPERTY()
	TObjectPtr<UAOExperienceManagerComponent> AOGameStateComponent;
	
	UPROPERTY()
	TObjectPtr<UAOPlayerSpawningManagerComponent> APlayerSpawningManagerComponent;
	
	// AI Bot创建组件
	UPROPERTY()
	TObjectPtr<UAOAIBotCreationComponent> AIBotCreationComponent;
};
