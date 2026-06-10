#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "AegisOdyssey/Inventory/AOInventoryItemDefinition.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_ItemHoverTooltip.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_Crafting.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVVMCraftingSameSelectionRefreshesTest,
	"AegisOdyssey.Crafting.MVVM.SameSelectionRefreshes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVVMCraftingExposesRequestFeedbackStateTest,
	"AegisOdyssey.Crafting.MVVM.ExposesRequestFeedbackState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAOCraftingQueueEntryViewDataExposesBatchProgressFieldsTest,
	"AegisOdyssey.Crafting.QueueView.ExposesBatchProgressFields",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMVVMItemHoverTooltipTracksLatestSourceTokenTest,
	"AegisOdyssey.Inventory.Tooltip.TracksLatestSourceToken",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryItemDefinitionExposesDescriptionFieldTest,
	"AegisOdyssey.Inventory.Definition.ExposesDescriptionField",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMVVMCraftingSameSelectionRefreshesTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UMVVM_Crafting* ViewModel = NewObject<UMVVM_Crafting>(GetTransientPackage());
	TestNotNull(TEXT("应能创建制造 ViewModel"), ViewModel);
	if (ViewModel == nullptr)
	{
		return false;
	}

	int32 ObservationChangedBroadcastCount = 0;
	ViewModel->OnCraftingObservationChanged.AddLambda([&ObservationChangedBroadcastCount]()
	{
		++ObservationChangedBroadcastCount;
	});

	FProperty* SelectedRecipeRowNameProperty = ViewModel->GetClass()->FindPropertyByName(TEXT("SelectedRecipeRowName"));
	TestNotNull(TEXT("应能通过反射找到 SelectedRecipeRowName"), SelectedRecipeRowNameProperty);
	if (SelectedRecipeRowNameProperty == nullptr)
	{
		return false;
	}

	const FName SameRecipeRowName = TEXT("Recipe_A");
	SelectedRecipeRowNameProperty->ImportText_Direct(
		*SameRecipeRowName.ToString(),
		SelectedRecipeRowNameProperty->ContainerPtrToValuePtr<void>(ViewModel),
		ViewModel,
		PPF_None);

	ViewModel->SetSelectedRecipeRowName(SameRecipeRowName);

	TestEqual(
		TEXT("再次点中同一配方时，仍应触发一次显式刷新广播"),
		ObservationChangedBroadcastCount,
		1);

	return true;
}

bool FMVVMCraftingExposesRequestFeedbackStateTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const UClass* ViewModelClass = UMVVM_Crafting::StaticClass();
	TestNotNull(TEXT("应能获取 UMVVM_Crafting 类型"), ViewModelClass);
	if (ViewModelClass == nullptr)
	{
		return false;
	}

	TestNotNull(
		TEXT("ViewModel 应暴露 LastCraftRequestFeedback 供 UI 读取最近一次制作反馈"),
		ViewModelClass->FindPropertyByName(TEXT("LastCraftRequestFeedback")));

	TestNotNull(
		TEXT("ViewModel 应暴露 bHasCraftRequestFeedback 供 UI 判断当前是否有制作反馈"),
		ViewModelClass->FindPropertyByName(TEXT("bHasCraftRequestFeedback")));

	return true;
}

bool FAOCraftingQueueEntryViewDataExposesBatchProgressFieldsTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const UScriptStruct* QueueEntryViewDataStruct = FAOCraftingQueueEntryViewData::StaticStruct();
	TestNotNull(TEXT("应能获取 FAOCraftingQueueEntryViewData 结构体"), QueueEntryViewDataStruct);
	if (QueueEntryViewDataStruct == nullptr)
	{
		return false;
	}

	TestNotNull(
		TEXT("队列视图数据应暴露 TotalCraftCount 字段"),
		QueueEntryViewDataStruct->FindPropertyByName(TEXT("TotalCraftCount")));

	TestNotNull(
		TEXT("队列视图数据应暴露 RemainingCraftCount 字段"),
		QueueEntryViewDataStruct->FindPropertyByName(TEXT("RemainingCraftCount")));

	return true;
}

bool FMVVMItemHoverTooltipTracksLatestSourceTokenTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UMVVM_ItemHoverTooltip* TooltipViewModel = NewObject<UMVVM_ItemHoverTooltip>(GetTransientPackage());
	UAOInventoryItemDefinition* ItemDefinition = NewObject<UAOInventoryItemDefinition>(GetTransientPackage());
	UAOInventoryItemDefinition* SourceA = NewObject<UAOInventoryItemDefinition>(GetTransientPackage());
	UAOInventoryItemDefinition* SourceB = NewObject<UAOInventoryItemDefinition>(GetTransientPackage());

	TestNotNull(TEXT("应能创建 Tooltip ViewModel"), TooltipViewModel);
	TestNotNull(TEXT("应能创建测试用 ItemDefinition"), ItemDefinition);
	TestNotNull(TEXT("应能创建第一个悬浮来源"), SourceA);
	TestNotNull(TEXT("应能创建第二个悬浮来源"), SourceB);
	if (TooltipViewModel == nullptr || ItemDefinition == nullptr || SourceA == nullptr || SourceB == nullptr)
	{
		return false;
	}

	ItemDefinition->DisplayName = TEXT("Hover_Item");
	ItemDefinition->Description = FText::FromString(TEXT("完整描述"));

	TooltipViewModel->ShowTooltip(ItemDefinition, FVector2D(10.0f, 20.0f), SourceA);
	TestTrue(TEXT("显示后 Tooltip 应进入可见状态"), TooltipViewModel->IsTooltipVisible());
	TestEqual(TEXT("Tooltip 应显示当前物品名"), TooltipViewModel->GetItemDisplayName().ToString(), FString(TEXT("Hover_Item")));
	TestEqual(TEXT("Tooltip 应显示完整描述原文"), TooltipViewModel->GetItemDescription().ToString(), FString(TEXT("完整描述")));

	TooltipViewModel->ShowTooltip(ItemDefinition, FVector2D(30.0f, 40.0f), SourceB);
	TooltipViewModel->HideTooltip(SourceA);

	TestTrue(TEXT("旧来源离开时，不应误关掉已被新来源顶替的 Tooltip"), TooltipViewModel->IsTooltipVisible());

	TooltipViewModel->HideTooltip(SourceB);
	TestFalse(TEXT("当前活动来源离开时，Tooltip 应关闭"), TooltipViewModel->IsTooltipVisible());

	return true;
}

bool FInventoryItemDefinitionExposesDescriptionFieldTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const UClass* ItemDefinitionClass = UAOInventoryItemDefinition::StaticClass();
	TestNotNull(TEXT("应能获取 UAOInventoryItemDefinition 类型"), ItemDefinitionClass);
	if (ItemDefinitionClass == nullptr)
	{
		return false;
	}

	TestNotNull(
		TEXT("物品 Definition 应暴露 Description 字段供 Tooltip 读取完整描述"),
		ItemDefinitionClass->FindPropertyByName(TEXT("Description")));

	return true;
}

#endif
