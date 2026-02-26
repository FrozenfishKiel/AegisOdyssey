// Fill out your copyright notice in the Description page of Project Settings.


#include "AOInputBufferComponent.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AOExtPawnComponent.h"
#include "AOHeroComponent.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInputBufferComponent)


UAOInputBufferComponent::UAOInputBufferComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	SaveInputType = EInputType::None;
	SaveInputTag = FGameplayTag();
	InputTimestamp = 0.0f;
}

void UAOInputBufferComponent::SetBufferedInput(const FGameplayTag& InputTag, const EInputType InputType)
{
	SaveInputTag = InputTag;
	SaveInputType = InputType;
	InputTimestamp = GetWorld()->GetTimeSeconds();  //获取当前时间
	UE_LOG(LogAegisOdysseyPlayer,Warning, TEXT("储存了当前的预输入变量 "));

}

FGameplayTag UAOInputBufferComponent::GetBufferedInput() const
{
	return SaveInputTag;
}

EInputType UAOInputBufferComponent::GetBufferedInputType() const
{
	return SaveInputType;
}

//清除预输入记录
void UAOInputBufferComponent::ClearBufferedInput()
{
	SaveInputTag = FGameplayTag();
	SaveInputType = EInputType::None;
	InputTimestamp = 0.0f;
}

//计算过了多长时间，如果超出预输入限制记录时间则失效
inline bool UAOInputBufferComponent::IsBufferedInputValid() const
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float ElapsedTime = CurrentTime - InputTimestamp;
    
	return ElapsedTime < BufferDuration;
}

bool UAOInputBufferComponent::TriggerBufferedInput()
{
	if (!SaveInputTag.IsValid() && SaveInputType == EInputType::None) return false;
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const UAOExtPawnComponent* PawnExtComp = UAOExtPawnComponent::FindAOExtPawnComponent(Pawn))
		{
			if (UAOAbilitySystem* AOASC = PawnExtComp->GetAOAbilitySystemComponent())
			{
				if (SaveInputType == EInputType::Trigger)
				{
					AOASC->AbilityInputTagPressed(SaveInputTag);
				}
				else if (SaveInputType == EInputType::Start)
				{
					AOASC->AbilityInputTagStarted(SaveInputTag);
				}
				else
				{
					AOASC->AbilityInputTagReleased(SaveInputTag);
				}
			}
		}
	}
	return false;
}

// Called when the game starts
void UAOInputBufferComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UAOInputBufferComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//  如果过了有效的预输入储存时间，则自动清除
	if (!IsBufferedInputValid())
	{
		ClearBufferedInput();
	}
}

