// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonPlayerController.h"
#include "AegisOdyssey/AOCombatResultMessage.h"
#include "AOPlayerController.generated.h"

class UAOInteractionSessionComponent;
class UAOLocalTargetHealthBarObserverComponent;

// 项目自定义的玩家控制器。
// 它持有玩家私有的交互会话与本地目标血条观察组件，并提供少量客户端专属桥接入口。
UCLASS()
class AEGISODYSSEY_API AAOPlayerController : public ACommonPlayerController
{
	GENERATED_BODY()

public:
	AAOPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 制造系统现阶段保留的最小调试入口。
	// 它只负责把控制台输入转成一次配方入队请求，不承担正式 UI 或交互入口职责。
	UFUNCTION(Exec)
	void CraftRecipe(FName RecipeRowName);

	// 开发期调试发货入口。
	// 它只负责把控制台输入转成一次服务端权威入包请求，不另起第二条库存主链。
	UFUNCTION(Exec)
	void GiveItem(int32 ItemId, int32 Count, FString TargetActorNameOrPath = TEXT(""));

	UFUNCTION(Exec)
	void DebugOpenCharacterInventory(FString TargetActorNameOrPath = TEXT(""));

	// 在控制器输入后处理中驱动 ASC 的输入消费流程。
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

	// 把本地客户端发起的一次统一交互请求转发到服务端权威执行。
	UFUNCTION(Server, Reliable)
	void Server_ExecuteInteractionRequest(AActor* TargetActor, FName TargetComponentName, int32 OptionIndex);

	// 客户端输入的调试发货命令，服务端负责真正解析目标与执行入包。
	UFUNCTION(Server, Reliable)
	void Server_GiveItemRequest(int32 ItemId, int32 Count, const FString& TargetActorNameOrPath);

	UFUNCTION(Server, Reliable)
	void Server_DebugOpenCharacterInventoryRequest(const FString& TargetActorNameOrPath);

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	UAOInteractionSessionComponent* GetInteractionSessionComponent() const { return InteractionSessionComponent; }

	// 服务端把和本地玩家相关的正式战斗结果定向转发到这里。
	// 客户端收到后只在本地 CombatMessageSubsystem 上复播，不参与二次判定。
	UFUNCTION(Client, Reliable)
	void ClientBroadcastCombatResultMessage(const FAOCombatResultMessage& Message);

	// 把本次调试发货的成功或失败结果回传给输入命令的客户端。
	UFUNCTION(Client, Reliable)
	void ClientReportGiveItemResult(bool bSucceeded, const FString& ResultMessage);

	UFUNCTION(BlueprintPure, Category = "AO|UI")
	UAOLocalTargetHealthBarObserverComponent* GetLocalTargetHealthBarObserverComponent() const { return LocalTargetHealthBarObserverComponent; }

protected:
	// 玩家当前交互会话的承载组件。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|Interaction")
	TObjectPtr<UAOInteractionSessionComponent> InteractionSessionComponent = nullptr;

	// 本地玩家视角下的目标血条观察组件。
	// 它只负责决定“我现在要不要看到这个目标血条”，不维护目标自己的生命值真相。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AO|UI")
	TObjectPtr<UAOLocalTargetHealthBarObserverComponent> LocalTargetHealthBarObserverComponent = nullptr;

private:
	bool TryExecuteGiveItemOnAuthority(int32 ItemId, int32 Count, const FString& TargetActorNameOrPath, FString& OutResultMessage);
	bool TryExecuteDebugOpenCharacterInventoryOnAuthority(const FString& TargetActorNameOrPath, FString& OutResultMessage);
};
