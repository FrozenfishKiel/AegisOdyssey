// Fill out your copyright notice in the Description page of Project Settings.

#include "AOAILogicStateTreeComponentBase.h"

#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/Character/Enemies/AI/Decision/AOAIDecisionComponent.h"
#include "StateTreeEvents.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOAILogicStateTreeComponentBase)

UAOAILogicStateTreeComponentBase::UAOAILogicStateTreeComponentBase()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
}

void UAOAILogicStateTreeComponentBase::BeginPlay()
{
	Super::BeginPlay();
	BindInventoryDecisionEvents();
}

void UAOAILogicStateTreeComponentBase::ApplyDefaultStateTreeIfNeeded()
{
	if (GetStateTreeAsset() != nullptr || DefaultStateTree == nullptr)
	{
		return;
	}

	SetStateTree(DefaultStateTree);
}

void UAOAILogicStateTreeComponentBase::InitializeComponent()
{
	ApplyDefaultStateTreeIfNeeded();
	Super::InitializeComponent();
	BindInventoryDecisionEvents();
}

void UAOAILogicStateTreeComponentBase::UninitializeComponent()
{
	UnbindInventoryDecisionEvents();
	Super::UninitializeComponent();
	StopLogic(FString("None"));
}

void UAOAILogicStateTreeComponentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindInventoryDecisionEvents();
	Super::EndPlay(EndPlayReason);
	StopLogic(FString("None"));
}

void UAOAILogicStateTreeComponentBase::FullReset()
{
	InstanceData.Reset();
}

void UAOAILogicStateTreeComponentBase::BindInventoryDecisionEvents()
{
	// StateTree 桥接层现在只订阅“正式提交的库存结果”。
	// 这样运行中的树收到的就是统一主链已经确认过的结果，而不是评估期中间态。
	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr)
	{
		return;
	}

	UAOAIDecisionComponent* DecisionComponent = UAOAIDecisionComponent::FindAIDecisionComponent(OwnerActor);
	if (CachedDecisionComponent == DecisionComponent && SubmittedInventoryDecisionChangedHandle.IsValid())
	{
		return;
	}

	UnbindInventoryDecisionEvents();

	CachedDecisionComponent = DecisionComponent;
	if (CachedDecisionComponent == nullptr)
	{
		return;
	}

	SubmittedInventoryDecisionChangedHandle = CachedDecisionComponent->OnSubmittedInventoryDecisionChanged().AddUObject(
		this,
		&ThisClass::HandleSubmittedInventoryDecisionChanged);
}

void UAOAILogicStateTreeComponentBase::UnbindInventoryDecisionEvents()
{
	if (CachedDecisionComponent != nullptr && SubmittedInventoryDecisionChangedHandle.IsValid())
	{
		CachedDecisionComponent->OnSubmittedInventoryDecisionChanged().Remove(SubmittedInventoryDecisionChangedHandle);
	}

	SubmittedInventoryDecisionChangedHandle.Reset();
	CachedDecisionComponent = nullptr;
}

void UAOAILogicStateTreeComponentBase::HandleSubmittedInventoryDecisionChanged(const FAOAIInventoryDecisionResult& SubmittedInventoryDecision)
{
	// 统一把库存结果变化转成 StateTree Event。
	// 有动作时发 Updated，没有动作时发 Cleared，方便资源层只监听事件语义。
	FStateTreeEvent Event;
	Event.Tag = SubmittedInventoryDecision.bHasAction
		? AOGameplayTags::AI_Event_InventoryDecision_Updated
		: AOGameplayTags::AI_Event_InventoryDecision_Cleared;

	FInstancedStruct Payload;
	Payload.InitializeAs<FAOAIInventoryDecisionResult>(SubmittedInventoryDecision);
	Event.Payload = FConstStructView(Payload);

	SendStateTreeEvent(Event);
}

void UAOAILogicStateTreeComponentBase::TickComponent(float DeltaTime, ELevelTick TickType,
                                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
