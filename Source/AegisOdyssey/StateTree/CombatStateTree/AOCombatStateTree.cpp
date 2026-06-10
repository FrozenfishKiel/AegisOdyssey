// Fill out your copyright notice in the Description page of Project Settings.


#include "AOCombatStateTree.h"

#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "AegisOdyssey/Character/AOInputBufferComponent.h"
#include "StateTreeEvents.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCombatStateTree)

UAOCombatStateTree::UAOCombatStateTree()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	bStartLogicAutomatically = true;
}

void UAOCombatStateTree::BeginPlay()
{
	Super::BeginPlay();
}

void UAOCombatStateTree::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAOCombatStateTree::OnRegister()
{
	Super::OnRegister();
}

void UAOCombatStateTree::InitializeComponent()
{
	Super::InitializeComponent();

	if (GetStateTreeAsset() == nullptr)
	{
		UE_LOG(LogStateTree, Warning, TEXT("%s has no StateTree asset configured on the component template."), *GetPathName());
	}

	if (UAOHeroComponent* HeroComponent = UAOHeroComponent::FindHeroComponent(GetOwner()))
	{
		OnPressInputLoadHandle = HeroComponent->OnPressInputLoad.Add(FOnPressInputLoad::FDelegate::CreateUObject(this,&ThisClass::CallStateTreeToSentEvent));
		OnReleaseInputLoadHandle = HeroComponent->OnReleaseInputLoad.Add(FOnReleaseInputLoad::FDelegate::CreateUObject(this,&ThisClass::CallStateTreeToSentEvent));
		OnStartInputLoadHandle = HeroComponent->OnStartInputLoad.Add(FOnStartInputLoad::FDelegate::CreateUObject(this,&ThisClass::CallStateTreeToSentEvent));
	}
	if (UAOInputBufferComponent* InputBufferComponent = UAOInputBufferComponent::FindOInputBufferComponent(GetOwner()))
	{
		OnPressInputBufferHandle = InputBufferComponent->OnPressInputBuffer.Add(FOnPressInputBuffer::FDelegate::CreateUObject(this,&ThisClass::CallStateTreeToSentEvent));
		OnStartInputBufferHandle = InputBufferComponent->OnStartInputBuffer.Add(FOnStartInputBuffer::FDelegate::CreateUObject(this,&ThisClass::CallStateTreeToSentEvent));
		OnReleaseInputBufferHandle = InputBufferComponent->OnReleaseInputBuffer.Add(FOnStartInputBuffer::FDelegate::CreateUObject(this,&ThisClass::CallStateTreeToSentEvent));
	}
}

void UAOCombatStateTree::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (UAOHeroComponent* HeroComponent = UAOHeroComponent::FindHeroComponent(GetOwner()))
	{
		if (OnPressInputLoadHandle.IsValid())
		{
			HeroComponent->OnPressInputLoad.Remove(OnPressInputLoadHandle);

		}
		if (OnReleaseInputLoadHandle.IsValid())
		{
			HeroComponent->OnReleaseInputLoad.Remove(OnReleaseInputLoadHandle);

		}
		if (OnStartInputLoadHandle.IsValid())
		{
			HeroComponent->OnStartInputLoad.Remove(OnStartInputLoadHandle);
		}
	}
	if (UAOInputBufferComponent* InputBufferComponent = UAOInputBufferComponent::FindOInputBufferComponent(GetOwner()))
	{
		if (OnPressInputBufferHandle.IsValid())
		{
			InputBufferComponent->OnPressInputBuffer.Remove(OnPressInputBufferHandle);
		}
		if (OnReleaseInputBufferHandle.IsValid())
		{
			InputBufferComponent->OnReleaseInputBuffer.Remove(OnReleaseInputBufferHandle);
		}
		if (OnStartInputBufferHandle.IsValid())
		{
			InputBufferComponent->OnStartInputBuffer.Remove(OnStartInputBufferHandle);
		}
	}
}

void UAOCombatStateTree::FullReset()
{
	Super::FullReset();
	InstanceData.Reset();
}

void UAOCombatStateTree::CallStateTreeToSentEvent(const FGameplayTag InTargetTag, const EInputType InInputType)
{
	// Hero 的 Press 会在按住期间持续广播。
	// 这里不改 Hero 发送职责，而是在 StateTree 消费侧统一把“同一次按住”收敛成一次命令。
	UE_LOG(LogStateTree, Verbose, TEXT("CallStateTreeToSentEvent: Tag=%s, InputType=%d"), *InTargetTag.ToString(), (int32)InInputType);
	FStateTreeEvent Event;
	
	FInstancedStruct Payload;
	Payload.InitializeAs<FCombatStateTreeInputEvent>(FCombatStateTreeInputEvent(InTargetTag, InInputType));
	Event.Payload = FConstStructView(Payload);
	Event.Tag = InTargetTag;
	
	SendStateTreeEvent(Event);
	return;
}
