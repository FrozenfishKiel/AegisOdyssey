// Fill out your copyright notice in the Description page of Project Settings.

#include "AOAbilitySystem.h"

#include "AOGlobalAbilitySystem.h"
#include "NativeGameplayTags.h"
#include "Abilities/AOGameplayAbility.h"
#include "AegisOdyssey/Animation/AOAnimInstance.h"
#include "AttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOAbilitySystem)
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_AbilityInputBlocked, "Gameplay.AbilityInputBlocked");

void UAOAbilitySystem::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);

	const bool bHasNewPawnActor = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);

	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

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
}

UAttributeSet* UAOAbilitySystem::EnsureSpawnedAttributeSet(TSubclassOf<UAttributeSet> AttributeSetClass)
{
	if (!IsValid(AttributeSetClass))
	{
		return nullptr;
	}

	// AbilitySet 在授予时需要确保依赖的 AttributeSet 已经挂到当前 ASC 上。
	// 这里做的不是另一条独立配置链，而只是一个“按类确保存在”的最小辅助：
	// 已存在就复用，不存在才补建并注册一次。
	if (const UAttributeSet* ExistingAttributeSet = GetAttributeSubobject(AttributeSetClass))
	{
		return const_cast<UAttributeSet*>(ExistingAttributeSet);
	}

	UAttributeSet* NewAttributeSet = NewObject<UAttributeSet>(GetOwner(), AttributeSetClass);
	AddAttributeSetSubobject(NewAttributeSet);
	return NewAttributeSet;
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

void UAOAbilitySystem::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);
	AbilitySystemDataChangedDelegate.Broadcast();
}

void UAOAbilitySystem::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();
	AbilitySystemDataChangedDelegate.Broadcast();
}

void UAOAbilitySystem::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	TArray<FGameplayAbilitySpecHandle> MatchingHandles;
	GatherAbilityHandlesForInputTag(InputTag, MatchingHandles);

	for (const FGameplayAbilitySpecHandle& AbilityHandle : MatchingHandles)
	{
		InputPressedSpecHandles.AddUnique(AbilityHandle);
		InputHeldSpecHandles.AddUnique(AbilityHandle);
	}
}

void UAOAbilitySystem::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	TArray<FGameplayAbilitySpecHandle> MatchingHandles;
	GatherAbilityHandlesForInputTag(InputTag, MatchingHandles);

	for (const FGameplayAbilitySpecHandle& AbilityHandle : MatchingHandles)
	{
		InputReleasedSpecHandles.AddUnique(AbilityHandle);
		InputHeldSpecHandles.Remove(AbilityHandle);
	}
}

void UAOAbilitySystem::AbilityInputTagStarted(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	TArray<FGameplayAbilitySpecHandle> MatchingHandles;
	GatherAbilityHandlesForInputTag(InputTag, MatchingHandles);

	for (const FGameplayAbilitySpecHandle& AbilityHandle : MatchingHandles)
	{
		InputStartedSpecHandles.AddUnique(AbilityHandle);
	}
}

void UAOAbilitySystem::GatherAbilityHandlesForInputTag(
	const FGameplayTag& InputTag,
	TArray<FGameplayAbilitySpecHandle>& OutAbilityHandles) const
{
	OutAbilityHandles.Reset();

	if (!InputTag.IsValid())
	{
		return;
	}

	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.Ability || !AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}

		OutAbilityHandles.AddUnique(AbilitySpec.Handle);
	}
}

void UAOAbilitySystem::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
	FPredictionKey OriginalPredictionKey = Instance
		? Instance->GetCurrentActivationInfo().GetActivationPredictionKey()
		: Spec.ActivationInfo.GetActivationPredictionKey();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, OriginalPredictionKey);
}

void UAOAbilitySystem::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	if (Spec.IsActive())
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance
			? Instance->GetCurrentActivationInfo().GetActivationPredictionKey()
			: Spec.ActivationInfo.GetActivationPredictionKey();
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, OriginalPredictionKey);
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

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					AbilitySpecInputPressed(*AbilitySpec);
				}
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

				if (AbilitySpec->IsActive())
				{
					AbilitySpecInputPressed(*AbilitySpec);
				}
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

	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}

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
