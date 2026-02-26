// Fill out your copyright notice in the Description page of Project Settings.


#include "AOAbilitySystem.h"

#include "AOGlobalAbilitySystem.h"
#include "NativeGameplayTags.h"
#include "Abilities/AOGameplayAbility.h"
#include "AegisOdyssey/Animation/AOAnimInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOAbilitySystem)
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_AbilityInputBlocked, "Gameplay.AbilityInputBlocked");


void UAOAbilitySystem::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{

	FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);
	//确保当前的AvatarActor和传入的AvatarActor不是同一个

	const bool bHasNewPawnActor = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);
	
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	//切换玩家的时候
	if (bHasNewPawnActor)
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
			for (UGameplayAbility* AbilityInstance : Instances)
			{
				UAOGameplayAbility* AOAbilityInstance = Cast<UAOGameplayAbility>(AbilityInstance);
				if (AOAbilityInstance)
				{
					
				}
			}
			if (UAOGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UAOGlobalAbilitySystem>(GetWorld()))
			{
				
			}
			if (UAOAnimInstance* AOAnimInstance = Cast<UAOAnimInstance>(ActorInfo->AnimInstance))
			{
				AOAnimInstance->InitializeWithAbilitySystem(this);
			}
		}
		
	}
	TryActivateAbilitiesOnSpawn();  //尝试激活被GiveAbility的OnSpawn类型的技能
}

void UAOAbilitySystem::TryActivateAbilitiesOnSpawn()
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (const UAOGameplayAbility* AOAbilityCDO = Cast<UAOGameplayAbility>(AbilitySpec.Ability))
		{
			AOAbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
		}
	}
}



void UAOAbilitySystem::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void UAOAbilitySystem::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}
}

void UAOAbilitySystem::AbilityInputTagStarted(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputStartedSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void UAOAbilitySystem::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	//控制编译器提示代码过时的宏
	//使用复制事件，以便 WaitlnputPress 能力任务可以正常工作。
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
	FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	//调用InputPressed事件。这里没有复制。如果有人正在监听，他们可能会将InputPressed事件复制到服务器。
	InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed,Spec.Handle,OriginalPredictionKey);
}

void UAOAbilitySystem::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	if (Spec.IsActive())
	{
		//控制编译器提示代码过时的宏
		//使用复制事件，以便 WaitlnputReleased 能力任务可以正常工作。
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

		//调用InputPressed事件。这里没有复制。如果有人正在监听，他们可能会将InputPressed事件复制到服务器。
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased,Spec.Handle,OriginalPredictionKey);
	}
}

void UAOAbilitySystem::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
		if (HasMatchingGameplayTag(TAG_Gameplay_AbilityInputBlocked))
	{
		ClearAbilityInput();
		return;
	}

	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();
	

	//
	// Process all abilities that activate when the input is held.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UAOGameplayAbility* AOAbilityCDO = Cast<UAOGameplayAbility>(AbilitySpec->Ability);
				if (AOAbilityCDO && AOAbilityCDO->GetActivationPolicy() == EAOAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}

	//
	// Process all abilities that had their input pressed this frame.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				//如果Ability已经激活，则发送Pressed事件
				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputPressed(*AbilitySpec);
				}
				//如果Ability没有激活，则通过按键激活
				else
				{
					const UAOGameplayAbility* AOAbilityCDO = Cast<UAOGameplayAbility>(AbilitySpec->Ability);

					if (AOAbilityCDO && AOAbilityCDO->GetActivationPolicy() == EAOAbilityActivationPolicy::OnInputTriggered)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputStartedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				//如果Ability已经激活，则发送Pressed事件
				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputPressed(*AbilitySpec);
				}
				//如果Ability没有激活，则通过按键激活
				else
				{
					const UAOGameplayAbility* AOAbilityCDO = Cast<UAOGameplayAbility>(AbilitySpec->Ability);

					if (AOAbilityCDO && AOAbilityCDO->GetActivationPolicy() == EAOAbilityActivationPolicy::Start)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}

	//
	//尝试激活所有来自点击和按下的能力。我们一次完成所有操作，这样按下的输入不会激活能力，然后也会因为点击而向能力发送输入事件。
	//
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	//
	// Process all abilities that had their input released this frame.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}

	//
	// Clear the cached ability handles.
	//
	//按下后立刻清空Press池子，相当于结束当前press事件
	//但Hold还保留，在Released触发前，Hold的池子都不会被清空，只要它内部有成员，就会一直尝试激活对应的Ability
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputStartedSpecHandles.Reset();
}

void UAOAbilitySystem::ClearAbilityInput()
{
	InputStartedSpecHandles.Reset();
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

