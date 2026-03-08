// Fill out your copyright notice in the Description page of Project Settings.


#include "AOCombatLocomotionStateTree.h"

#include "AOCombatStateTree.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "AegisOdyssey/Character/AOInputBufferComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCombatLocomotionStateTree)


// Sets default values for this component's properties
UAOCombatLocomotionStateTree::UAOCombatLocomotionStateTree()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	// ...
}


// Called when the game starts
void UAOCombatLocomotionStateTree::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UAOCombatLocomotionStateTree::InitializeComponent()
{
	Super::InitializeComponent();
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

void UAOCombatLocomotionStateTree::UninitializeComponent()
{
	Super::UninitializeComponent();
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


void UAOCombatLocomotionStateTree::CallStateTreeToSentEvent(const FGameplayTag InTargetTag,
	const EInputType InInputType)
{
	if (!InTargetTag.IsValid())
	{
		return ;
	}
	UE_LOG(LogStateTree, Warning, TEXT("CallStateTreeToSentEvent: Tag=%s, InputType=%d"), *InTargetTag.ToString(), (int32)InInputType);
	FStateTreeEvent Event;
	
	FInstancedStruct Payload;
	Payload.InitializeAs<FCombatStateTreeInputEvent>(FCombatStateTreeInputEvent(InTargetTag, InInputType));
	Event.Payload = FConstStructView(Payload);
	Event.Tag = InTargetTag;
	
	SendStateTreeEvent(Event);
}

// Called every frame
void UAOCombatLocomotionStateTree::TickComponent(float DeltaTime, ELevelTick TickType,
                                                 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

