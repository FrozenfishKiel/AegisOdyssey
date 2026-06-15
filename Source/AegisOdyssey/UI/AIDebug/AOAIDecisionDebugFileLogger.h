#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

struct FAOAIDecisionDebugSnapshot;

// AI 决策调试文件输出器。
// 这条链路只服务调试观察，不参与正式 AI 决策与执行逻辑。
class FAOAIDecisionDebugFileLogger
{
public:
	// 返回 AI 调试日志目录。
	static FString GetLogDirectoryPath();

	// 为本轮调试会话创建一份按时间命名的 txt 日志文件路径。
	static FString CreateSessionLogFilePath();

	// 初始化一份新的调试会话日志文件，并写入会话头。
	static void InitializeSessionLogFile(const FString& LogFilePath);

	// 追加一帧 AI 决策调试快照到当前会话日志。
	static void AppendSnapshot(const FString& LogFilePath, const FAOAIDecisionDebugSnapshot& DebugSnapshot);

private:
	static FString BuildSnapshotText(const FAOAIDecisionDebugSnapshot& DebugSnapshot, const FDateTime& SnapshotTime);
	static FString BuildTagText(const FGameplayTag& GameplayTag);
};
