#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIDecisionQueueRejectsNonAuthorityMutationTest,
	"AegisOdyssey.AI.DecisionQueue.RejectsNonAuthorityMutation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIDecisionQueueKeepsFIFOAndDropsOverflowTest,
	"AegisOdyssey.AI.DecisionQueue.KeepsFIFOAndDropsOverflow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIDecisionQueueDoesNotSubmitWhenEmptyTest,
	"AegisOdyssey.AI.DecisionQueue.DoesNotSubmitWhenEmpty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

namespace
{
static UAOAIDecisionComponent* CreateTestDecisionComponent(FAutomationTestBase& Test, AAOCharacter*& OutCharacter)
{
	OutCharacter = NewObject<AAOCharacter>(GetTransientPackage());
	Test.TestNotNull(TEXT("应能创建 AI 决策测试角色。"), OutCharacter);
	if (OutCharacter == nullptr)
	{
		return nullptr;
	}

	UAOAIDecisionComponent* DecisionComponent = OutCharacter->FindComponentByClass<UAOAIDecisionComponent>();
	Test.TestNotNull(TEXT("测试角色应挂载 AI 决策组件。"), DecisionComponent);
	return DecisionComponent;
}
}

bool FAIDecisionQueueRejectsNonAuthorityMutationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	AAOCharacter* TestCharacter = nullptr;
	UAOAIDecisionComponent* DecisionComponent = CreateTestDecisionComponent(*this, TestCharacter);
	if (DecisionComponent == nullptr || TestCharacter == nullptr)
	{
		return false;
	}

	TestCharacter->SetRole(ROLE_SimulatedProxy);

	TestFalse(
		TEXT("非服务端权威角色不应允许入队新决策。"),
		DecisionComponent->EnqueueDecisionTag(AOGameplayTags::AI_Intent_Attack, 0.0f));

	FAOAIDecisionQueueItem SubmittedDecision;
	TestFalse(
		TEXT("非服务端权威角色不应允许推进提交流程。"),
		DecisionComponent->TrySubmitNextDecision(1.0f, SubmittedDecision));

	TestEqual(TEXT("非服务端权威路径不应持有队列内容。"), DecisionComponent->GetDecisionQueueCount(), 0);
	TestFalse(TEXT("非服务端权威路径不应出现待提交标签。"), DecisionComponent->HasPendingDecisionTag());

	return true;
}

bool FAIDecisionQueueKeepsFIFOAndDropsOverflowTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	AAOCharacter* TestCharacter = nullptr;
	UAOAIDecisionComponent* DecisionComponent = CreateTestDecisionComponent(*this, TestCharacter);
	if (DecisionComponent == nullptr)
	{
		return false;
	}

	const TArray<FGameplayTag> InputTags = {
		AOGameplayTags::AI_Intent_Attack,
		AOGameplayTags::AI_Intent_Strafe,
		AOGameplayTags::AI_Intent_Roll,
		AOGameplayTags::AI_Decision_Inventory_UseItem,
		AOGameplayTags::AI_Intent_Attack,
		AOGameplayTags::AI_Intent_Strafe
	};

	for (int32 Index = 0; Index < 5; ++Index)
	{
		TestTrue(
			FString::Printf(TEXT("第 %d 个合法决策应能成功入队。"), Index + 1),
			DecisionComponent->EnqueueDecisionTag(InputTags[Index], 0.0f));
	}

	TestFalse(
		TEXT("固定容量 5 的队列满后应直接丢弃新决策。"),
		DecisionComponent->EnqueueDecisionTag(InputTags[5], 0.0f));

	TestEqual(TEXT("队列容量应固定锁为 5。"), DecisionComponent->GetDecisionQueueCount(), 5);
	TestTrue(TEXT("队列在有内容时应暴露待提交标签状态。"), DecisionComponent->HasPendingDecisionTag());
	TestTrue(
		TEXT("当前队头决策标签应保持第一条入队结果。"),
		DecisionComponent->GetCurrentQueuedDecisionTag().MatchesTagExact(AOGameplayTags::AI_Intent_Attack));

	const float FirstSubmitTimeSeconds = DecisionComponent->GetNextQueuedDecisionSubmitTimeSeconds();
	TestTrue(TEXT("首个入队项应生成下一次提交时间。"), FirstSubmitTimeSeconds >= 0.0f);

	FAOAIDecisionQueueItem SubmittedDecision;
	TestFalse(
		TEXT("未到下一次提交时间前不应提前出队。"),
		DecisionComponent->TrySubmitNextDecision(FirstSubmitTimeSeconds - 0.01f, SubmittedDecision));

	TestTrue(
		TEXT("到达下一次提交时间后应只提交一个队头决策。"),
		DecisionComponent->TrySubmitNextDecision(FirstSubmitTimeSeconds, SubmittedDecision));
	TestTrue(TEXT("第一次提交必须保持 FIFO 队头顺序。"), SubmittedDecision.DecisionTag.MatchesTagExact(AOGameplayTags::AI_Intent_Attack));
	TestEqual(TEXT("单次提交后队列应只减少一项。"), DecisionComponent->GetDecisionQueueCount(), 4);
	TestTrue(
		TEXT("第一次提交后新的队头应切到第二条入队结果。"),
		DecisionComponent->GetCurrentQueuedDecisionTag().MatchesTagExact(AOGameplayTags::AI_Intent_Strafe));

	const TArray<FGameplayTag> ExpectedRemainingOrder = {
		AOGameplayTags::AI_Intent_Strafe,
		AOGameplayTags::AI_Intent_Roll,
		AOGameplayTags::AI_Decision_Inventory_UseItem,
		AOGameplayTags::AI_Intent_Attack
	};

	for (int32 Index = 0; Index < ExpectedRemainingOrder.Num(); ++Index)
	{
		const float CurrentTimeSeconds = 1.0f + static_cast<float>(Index);
		TestTrue(
			FString::Printf(TEXT("第 %d 次追加提交应成功。"), Index + 2),
			DecisionComponent->TrySubmitNextDecision(CurrentTimeSeconds, SubmittedDecision));
		TestTrue(
			FString::Printf(TEXT("第 %d 次提交仍应严格遵守 FIFO。"), Index + 2),
			SubmittedDecision.DecisionTag.MatchesTagExact(ExpectedRemainingOrder[Index]));
	}

	TestEqual(TEXT("队列内的第五项提交完成后应清空。"), DecisionComponent->GetDecisionQueueCount(), 0);
	TestFalse(TEXT("队列清空后不应再暴露待提交标签。"), DecisionComponent->HasPendingDecisionTag());
	TestFalse(
		TEXT("第六项溢出决策应在第一阶段被丢弃，而不是延后保留。"),
		DecisionComponent->TrySubmitNextDecision(10.0f, SubmittedDecision));

	return true;
}

bool FAIDecisionQueueDoesNotSubmitWhenEmptyTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	AAOCharacter* TestCharacter = nullptr;
	UAOAIDecisionComponent* DecisionComponent = CreateTestDecisionComponent(*this, TestCharacter);
	if (DecisionComponent == nullptr)
	{
		return false;
	}

	FAOAIDecisionQueueItem SubmittedDecision;
	TestFalse(
		TEXT("空队列不应提交任何决策。"),
		DecisionComponent->TrySubmitNextDecision(1.0f, SubmittedDecision));

	TestFalse(TEXT("空队列默认不应有待提交标签。"), DecisionComponent->HasPendingDecisionTag());
	TestFalse(TEXT("空队列的当前队头标签应保持无效。"), DecisionComponent->GetCurrentQueuedDecisionTag().IsValid());
	TestTrue(TEXT("空队列的下一次提交时间应保持未调度状态。"), DecisionComponent->GetNextQueuedDecisionSubmitTimeSeconds() < 0.0f);

	return true;
}

#endif
