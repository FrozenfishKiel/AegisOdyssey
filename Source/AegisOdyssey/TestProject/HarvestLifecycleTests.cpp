#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "TestHarvestLifecycleActors.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHarvestableActorBridgeAppliesCommonStateBeforeNativeHooksTest,
	"AegisOdyssey.Harvest.ActorBridge.AppliesCommonStateBeforeNativeHooks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHarvestableTreeKeepFallenRestoresCollisionAfterCommonBridgeTest,
	"AegisOdyssey.Harvest.Tree.KeepFallenRestoresCollisionAfterCommonBridge",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHarvestableActorBridgeAppliesCommonStateBeforeNativeHooksTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	ATestHarvestLifecycleActor* TestActor = NewObject<ATestHarvestLifecycleActor>(GetTransientPackage());
	TestNotNull(TEXT("应能创建采集 Actor 测试对象"), TestActor);
	if (TestActor == nullptr)
	{
		return false;
	}

	TestActor->InitializeForTest();

	UPrimitiveComponent* TestCollisionComponent = TestActor->GetTestCollisionComponent();
	TestNotNull(TEXT("测试 Actor 应包含碰撞组件"), TestCollisionComponent);
	if (TestCollisionComponent == nullptr)
	{
		return false;
	}

	TestEqual(TEXT("初始化后的碰撞状态应保留为 QueryOnly"), TestCollisionComponent->GetCollisionEnabled(), ECollisionEnabled::QueryOnly);

	FAOHarvestLifecycleContext LifecycleContext;
	TestActor->HandleHarvestNodeDepleted_Implementation(LifecycleContext);

	TestTrue(TEXT("公共 depleted 状态应先于 native 扩展点执行"), TestActor->WasDepletedNativeCalledWithCollisionDisabled());
	TestEqual(TEXT("depleted 后默认应退出全部 Primitive 碰撞"), TestCollisionComponent->GetCollisionEnabled(), ECollisionEnabled::NoCollision);

	TestActor->HandleHarvestNodeRespawned_Implementation();

	TestTrue(TEXT("公共 respawn 状态应先于 native 扩展点执行"), TestActor->WasRespawnNativeCalledWithCollisionRestored());
	TestEqual(TEXT("respawn 后应恢复初始碰撞快照"), TestCollisionComponent->GetCollisionEnabled(), ECollisionEnabled::QueryOnly);

	return true;
}

bool FHarvestableTreeKeepFallenRestoresCollisionAfterCommonBridgeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	ATestHarvestLifecycleTree* TestTree = NewObject<ATestHarvestLifecycleTree>(GetTransientPackage());
	TestNotNull(TEXT("应能创建树节点测试对象"), TestTree);
	if (TestTree == nullptr)
	{
		return false;
	}

	TestTree->InitializeForTest();

	UPrimitiveComponent* TreeBodyComponent = TestTree->GetTestTreeBodyComponent();
	TestNotNull(TEXT("树节点测试对象应包含主体碰撞体"), TreeBodyComponent);
	if (TreeBodyComponent == nullptr)
	{
		return false;
	}

	TestEqual(TEXT("初始化后的树主体碰撞应为 QueryAndPhysics"), TreeBodyComponent->GetCollisionEnabled(), ECollisionEnabled::QueryAndPhysics);
	TestEqual(TEXT("初始化后的采集可见性通道应阻挡"), TreeBodyComponent->GetCollisionResponseToChannel(ECC_Visibility), ECR_Block);

	FAOHarvestLifecycleContext LifecycleContext;
	TestTree->HandleHarvestNodeDepleted_Implementation(LifecycleContext);

	TestTrue(TEXT("Tree native 扩展点应接在公共 depleted 状态之后"), TestTree->WasTreeNativeCalledAfterCommonDepletedState());
	TestEqual(TEXT("KeepFallenTree 应在公共桥接之后恢复主体碰撞"), TreeBodyComponent->GetCollisionEnabled(), ECollisionEnabled::QueryAndPhysics);
	TestEqual(TEXT("KeepFallenTree depleted 后应退出采集命中链"), TreeBodyComponent->GetCollisionResponseToChannel(ECC_Visibility), ECR_Ignore);

	TestTree->HandleHarvestNodeRespawned_Implementation();

	TestEqual(TEXT("respawn 后树主体碰撞应恢复为 QueryAndPhysics"), TreeBodyComponent->GetCollisionEnabled(), ECollisionEnabled::QueryAndPhysics);
	TestEqual(TEXT("respawn 后采集可见性通道应重新阻挡"), TreeBodyComponent->GetCollisionResponseToChannel(ECC_Visibility), ECR_Block);

	return true;
}

#endif
