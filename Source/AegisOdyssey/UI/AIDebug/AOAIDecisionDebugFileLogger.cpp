#include "AegisOdyssey/UI/AIDebug/AOAIDecisionDebugFileLogger.h"

#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace AOAIDecisionDebugFileLoggerPrivate
{
	static FString BuildTimestampText(const FDateTime& Timestamp)
	{
		return FString::Printf(
			TEXT("%04d-%02d-%02d %02d:%02d:%02d.%03d"),
			Timestamp.GetYear(),
			Timestamp.GetMonth(),
			Timestamp.GetDay(),
			Timestamp.GetHour(),
			Timestamp.GetMinute(),
			Timestamp.GetSecond(),
			Timestamp.GetMillisecond());
	}
}

FString FAOAIDecisionDebugFileLogger::GetLogDirectoryPath()
{
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("AIDebug"));
}

FString FAOAIDecisionDebugFileLogger::CreateSessionLogFilePath()
{
	const FDateTime Now = FDateTime::Now();
	const FString FileName = FString::Printf(
		TEXT("AIDebugSession_%04d-%02d-%02d_%02d-%02d-%02d-%03d.txt"),
		Now.GetYear(),
		Now.GetMonth(),
		Now.GetDay(),
		Now.GetHour(),
		Now.GetMinute(),
		Now.GetSecond(),
		Now.GetMillisecond());
	return FPaths::Combine(GetLogDirectoryPath(), FileName);
}

void FAOAIDecisionDebugFileLogger::InitializeSessionLogFile(const FString& LogFilePath)
{
	if (LogFilePath.IsEmpty())
	{
		return;
	}

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(LogFilePath), true);

	const FDateTime Now = FDateTime::Now();
	const FString HeaderText = FString::Printf(
		TEXT("=== AI Decision Debug Session Start ===\n")
		TEXT("Session Time: %s\n")
		TEXT("Log File: %s\n")
		TEXT("\n"),
		*AOAIDecisionDebugFileLoggerPrivate::BuildTimestampText(Now),
		*LogFilePath);

	FFileHelper::SaveStringToFile(HeaderText, *LogFilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

void FAOAIDecisionDebugFileLogger::AppendSnapshot(const FString& LogFilePath, const FAOAIDecisionDebugSnapshot& DebugSnapshot)
{
	if (LogFilePath.IsEmpty())
	{
		return;
	}

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(LogFilePath), true);

	const FString SnapshotText = BuildSnapshotText(DebugSnapshot, FDateTime::Now());
	FFileHelper::SaveStringToFile(
		SnapshotText,
		*LogFilePath,
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,
		&IFileManager::Get(),
		FILEWRITE_Append);
}

FString FAOAIDecisionDebugFileLogger::BuildSnapshotText(const FAOAIDecisionDebugSnapshot& DebugSnapshot, const FDateTime& SnapshotTime)
{
	TArray<FString> Lines;
	Lines.Reserve(18);

	Lines.Add(TEXT("--------------------------------------------------"));
	Lines.Add(FString::Printf(TEXT("Snapshot Time: %s"), *AOAIDecisionDebugFileLoggerPrivate::BuildTimestampText(SnapshotTime)));
	Lines.Add(FString::Printf(TEXT("Tracking AI: %s"), DebugSnapshot.bIsTrackingAI ? TEXT("true") : TEXT("false")));
	Lines.Add(FString::Printf(TEXT("Tracked Actor: %s"), *DebugSnapshot.TrackedActorName.ToString()));
	Lines.Add(FString::Printf(TEXT("Selected Intent: %s"), *BuildTagText(DebugSnapshot.SelectedIntentTag)));
	Lines.Add(FString::Printf(TEXT("Has Evaluation Inventory Decision: %s"), DebugSnapshot.bHasCurrentEvaluationInventoryDecision ? TEXT("true") : TEXT("false")));
	Lines.Add(FString::Printf(TEXT("Evaluation Inventory Action: %s"), *BuildTagText(DebugSnapshot.CurrentEvaluationInventoryActionTag)));
	Lines.Add(FString::Printf(TEXT("Decision Queue Count: %d"), DebugSnapshot.DecisionQueueCount));
	Lines.Add(FString::Printf(TEXT("Current Queued Decision: %s"), *BuildTagText(DebugSnapshot.CurrentQueuedDecisionTag)));
	Lines.Add(FString::Printf(TEXT("Current Submitted Decision: %s"), *BuildTagText(DebugSnapshot.CurrentSubmittedDecisionTag)));
	Lines.Add(FString::Printf(TEXT("Last Submitted Decision: %s"), *BuildTagText(DebugSnapshot.LastSubmittedDecisionTag)));
	Lines.Add(FString::Printf(TEXT("Pending Submit Delay Seconds: %.3f"), DebugSnapshot.PendingSubmitDelaySeconds));
	Lines.Add(FString::Printf(TEXT("Has Submitted Inventory Decision: %s"), DebugSnapshot.bHasCurrentSubmittedInventoryDecision ? TEXT("true") : TEXT("false")));
	Lines.Add(FString::Printf(TEXT("Submitted Inventory Action: %s"), *BuildTagText(DebugSnapshot.CurrentSubmittedInventoryDecision.ActionTag)));
	Lines.Add(FString::Printf(TEXT("Submitted Inventory Candidate: %s"), *BuildTagText(DebugSnapshot.CurrentSubmittedInventoryDecision.CandidateTag)));
	Lines.Add(FString::Printf(TEXT("Submitted Inventory Desire: %.3f"), DebugSnapshot.CurrentSubmittedInventoryDecision.Desire));
	Lines.Add(FString::Printf(TEXT("Submitted Inventory Score: %.3f"), DebugSnapshot.CurrentSubmittedInventoryDecision.Score));
	Lines.Add(TEXT(""));

	return FString::Join(Lines, TEXT("\n"));
}

FString FAOAIDecisionDebugFileLogger::BuildTagText(const FGameplayTag& GameplayTag)
{
	return GameplayTag.IsValid() ? GameplayTag.GetTagName().ToString() : FString(TEXT("None"));
}
