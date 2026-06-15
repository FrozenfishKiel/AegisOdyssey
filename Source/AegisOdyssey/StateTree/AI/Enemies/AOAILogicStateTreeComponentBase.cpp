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
	bHasPendingSubmittedInventoryDecisionEvent = false;
	PendingSubmittedInventoryDecisionEvent = FAOAIInventoryDecisionResult();
	Super::UninitializeComponent();
	StopLogic(FString("None"));
}

void UAOAILogicStateTreeComponentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindInventoryDecisionEvents();
	bHasPendingSubmittedInventoryDecisionEvent = false;
	PendingSubmittedInventoryDecisionEvent = FAOAIInventoryDecisionResult();
	Super::EndPlay(EndPlayReason);
	StopLogic(FString("None"));
}

void UAOAILogicStateTreeComponentBase::FullReset()
{
	InstanceData.Reset();
	bHasPendingSubmittedInventoryDecisionEvent = false;
	PendingSubmittedInventoryDecisionEvent = FAOAIInventoryDecisionResult();
}

void UAOAILogicStateTreeComponentBase::BindInventoryDecisionEvents()
{
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

void UAOAILogicStateTreeComponentBase::HandleSubmittedInventoryDecisionChanged(
	const FAOAIInventoryDecisionResult& SubmittedInventoryDecision)
{
	if (!TryDispatchSubmittedInventoryDecisionEvent(SubmittedInventoryDecision))
	{
		bHasPendingSubmittedInventoryDecisionEvent = true;
		PendingSubmittedInventoryDecisionEvent = SubmittedInventoryDecision;
	}
}

bool UAOAILogicStateTreeComponentBase::TryDispatchSubmittedInventoryDecisionEvent(
	const FAOAIInventoryDecisionResult& SubmittedInventoryDecision)
{
	if (!IsRunning())
	{
		return false;
	}

	FStateTreeEvent Event;
	Event.Tag = SubmittedInventoryDecision.bHasAction
		? AOGameplayTags::AI_Event_InventoryDecision_Updated
		: AOGameplayTags::AI_Event_InventoryDecision_Cleared;

	FInstancedStruct Payload;
	Payload.InitializeAs<FAOAIInventoryDecisionResult>(SubmittedInventoryDecision);
	Event.Payload = FConstStructView(Payload);

	SendStateTreeEvent(Event);
	return true;
}

void UAOAILogicStateTreeComponentBase::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bHasPendingSubmittedInventoryDecisionEvent
		&& TryDispatchSubmittedInventoryDecisionEvent(PendingSubmittedInventoryDecisionEvent))
	{
		bHasPendingSubmittedInventoryDecisionEvent = false;
		PendingSubmittedInventoryDecisionEvent = FAOAIInventoryDecisionResult();
	}
}
