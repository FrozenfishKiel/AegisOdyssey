// Fill out your copyright notice in the Description page of Project Settings.

#include "AegisOdyssey/AOLogChannels.h"
#include "AegisOdyssey/StateTree/AI/Enemies/AOAILogicStateTreeComponentBase.h"
#include "AegisOdyssey/UI/AOHUD.h"
#include "AegisOdyssey/UI/AIDebug/AOAIDecisionDebugFileLogger.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"

namespace AOGlobalConsoleCommands
{
	// 解析控制台中的布尔开关参数，兼容 true/false、1/0、on/off。
	static bool ParseBoolArgument(const FString& InArgument, bool& OutEnabled)
	{
		if (InArgument.Equals(TEXT("true"), ESearchCase::IgnoreCase)
			|| InArgument.Equals(TEXT("1"), ESearchCase::IgnoreCase)
			|| InArgument.Equals(TEXT("on"), ESearchCase::IgnoreCase))
		{
			OutEnabled = true;
			return true;
		}

		if (InArgument.Equals(TEXT("false"), ESearchCase::IgnoreCase)
			|| InArgument.Equals(TEXT("0"), ESearchCase::IgnoreCase)
			|| InArgument.Equals(TEXT("off"), ESearchCase::IgnoreCase))
		{
			OutEnabled = false;
			return true;
		}

		return false;
	}

	// 全局切换当前世界中的 AI 决策 StateTree。
	// 这里只处理 UAOAILogicStateTreeComponentBase，避免误伤战斗、移动等其他 StateTree 组件。
	static void SetAIDecisionTreeEnabledForWorld(UWorld* World, const bool bEnabled)
	{
		if (World == nullptr)
		{
			UE_LOG(LogAegisOdyssey, Warning, TEXT("[GlobalConsole] SetDecisionTreeEnabled failed because World is null."));
			return;
		}

		int32 FoundComponentCount = 0;
		int32 UpdatedComponentCount = 0;
		int32 SkippedComponentCount = 0;

		for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
		{
			TInlineComponentArray<UAOAILogicStateTreeComponentBase*> StateTreeComponents(*ActorIt);
			for (UAOAILogicStateTreeComponentBase* StateTreeComponent : StateTreeComponents)
			{
				if (StateTreeComponent == nullptr)
				{
					continue;
				}

				++FoundComponentCount;

				if (bEnabled)
				{
					if (StateTreeComponent->GetStateTreeAsset() == nullptr)
					{
						++SkippedComponentCount;
						UE_LOG(
							LogAegisOdyssey,
							Warning,
							TEXT("[GlobalConsole] Skip enabling AI decision tree on [%s] because StateTreeAsset is null."),
							*GetNameSafe(StateTreeComponent->GetOwner()));
						continue;
					}

					StateTreeComponent->RestartLogic();
				}
				else
				{
					StateTreeComponent->StopLogic(TEXT("Disabled by global console command"));
				}

				++UpdatedComponentCount;
			}
		}

		UE_LOG(
			LogAegisOdyssey,
			Log,
			TEXT("[GlobalConsole] AI decision tree enabled=%s, updated=%d, found=%d, skipped=%d, world=%s"),
			bEnabled ? TEXT("true") : TEXT("false"),
			UpdatedComponentCount,
			FoundComponentCount,
			SkippedComponentCount,
			*GetNameSafe(World));
	}

	// 控制台命令入口：AegisOdyssey.AI.SetDecisionTreeEnabled <true|false>
	static void SetAIDecisionTreeEnabledCommand(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.Num() != 1)
		{
			UE_LOG(
				LogAegisOdyssey,
				Warning,
				TEXT("[GlobalConsole] Usage: AegisOdyssey.AI.SetDecisionTreeEnabled <true|false>"));
			return;
		}

		bool bEnabled = false;
		if (!ParseBoolArgument(Args[0], bEnabled))
		{
			UE_LOG(
				LogAegisOdyssey,
				Warning,
				TEXT("[GlobalConsole] Invalid argument [%s]. Usage: AegisOdyssey.AI.SetDecisionTreeEnabled <true|false>"),
				*Args[0]);
			return;
		}

		SetAIDecisionTreeEnabledForWorld(World, bEnabled);
	}

	static FAutoConsoleCommandWithWorldAndArgs CVarSetAIDecisionTreeEnabled(
		TEXT("AegisOdyssey.AI.SetDecisionTreeEnabled"),
		TEXT("全局开关当前世界中的 AI 决策 StateTree。用法: AegisOdyssey.AI.SetDecisionTreeEnabled <true|false>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&SetAIDecisionTreeEnabledCommand));

	static void SetAIDebugPanelEnabledCommand(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.Num() != 1)
		{
			UE_LOG(LogAegisOdyssey, Warning, TEXT("[GlobalConsole] Usage: AegisOdyssey.AI.SetDebugPanelEnabled <true|false>"));
			return;
		}

		bool bEnabled = false;
		if (!ParseBoolArgument(Args[0], bEnabled))
		{
			UE_LOG(
				LogAegisOdyssey,
				Warning,
				TEXT("[GlobalConsole] Invalid argument [%s]. Usage: AegisOdyssey.AI.SetDebugPanelEnabled <true|false>"),
				*Args[0]);
			return;
		}

		AAOHUD::SetAIDebugPanelEnabledForWorld(World, bEnabled);
		UE_LOG(
			LogAegisOdyssey,
			Log,
			TEXT("[GlobalConsole] AI debug panel enabled=%s, world=%s, log_dir=%s"),
			bEnabled ? TEXT("true") : TEXT("false"),
			*GetNameSafe(World),
			*FAOAIDecisionDebugFileLogger::GetLogDirectoryPath());
	}

	static FAutoConsoleCommandWithWorldAndArgs CVarSetAIDebugPanelEnabled(
		TEXT("AegisOdyssey.AI.SetDebugPanelEnabled"),
		TEXT("全局开关当前世界中的 AI 调试 Slate 面板，并在 Saved/AIDebug 下为本轮观察生成一份按时间命名的 txt 日志。用法: AegisOdyssey.AI.SetDebugPanelEnabled <true|false>"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&SetAIDebugPanelEnabledCommand));
}
