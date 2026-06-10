// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Sprint.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/ModMagCal/MMC_CalculateVigor.h"
#include "AegisOdyssey/Character/AOCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_Sprint)
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Sprint_Cost, "Ability.Sprint.Cost");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Sprint_CostVigorValue, "Ability.Sprint.CostVigorValue");

UGA_Sprint::UGA_Sprint(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	SprintSpeedBonusAmount = 0.f;
	bVigorExhaustedBroadcasted = false;
}

void UGA_Sprint::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                               const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

bool UGA_Sprint::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return false;
	}
	if (const UAOCombatAttributeSet* CombatAttribute = Cast<UAOCombatAttributeSet>(GetAbilitySystemComponentFromActorInfo()->GetAttributeSet(UAOCombatAttributeSet::StaticClass())))
	{
		float CurrentVigor = CombatAttribute->GetVigor();
		if (CurrentVigor <= 0.f) return false;
	}

	return true;
}

void UGA_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		UGameplayEffect* NewGE = NewObject<UGameplayEffect>(this,FName("VigorSprintEffect"));
		NewGE->DurationPolicy = EGameplayEffectDurationType::Infinite;
        
		FGameplayModifierInfo ModifierInfo;
		ModifierInfo.Attribute = UAOCombatAttributeSet::GetSprintSpeedBonusAttribute();
		ModifierInfo.ModifierOp = EGameplayModOp::Override;
		ModifierInfo.ModifierMagnitude = FScalableFloat(SprintSpeedBonusAmount);
        
		NewGE->Modifiers.Add(ModifierInfo);
        
		FGameplayEffectSpec Spec(NewGE, MakeEffectContext(Handle, ActorInfo));
		SprintSpeedEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(Spec);
		bVigorExhaustedBroadcasted = false;
		
		InitializeVigorCost();
	}

	if (!WaitInputReleaseTask)
	{
		WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
		WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGA_Sprint::OnInputReleased);
		WaitInputReleaseTask->ReadyForActivation();
	}
	if (!WaitVigorExhaustTask)
	{
		WaitVigorExhaustTask = UAT_WaitVigorExhaust::CreateWaitVigorExhaust(this);
		WaitVigorExhaustTask->OnWaitVigorExhaust.AddUObject(this, &UGA_Sprint::OnVigorWasExhausted);
		WaitVigorExhaustTask->OnCharacterMove.AddUObject(this,&UGA_Sprint::OnCharacterMoved);
		WaitVigorExhaustTask->OnCharacterStop.AddUObject(this,&ThisClass::OnCharacterStopped);
		WaitVigorExhaustTask->ReadyForActivation();
	}
	
}

void UGA_Sprint::OnInputReleased(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Sprint::OnVigorWasExhausted()
{
	//UE_LOG(LogAegisOdysseyAbilitySystem, Error, TEXT("Character Vigor Are Exhausted！！！！！！！！ "));
	bVigorExhaustedBroadcasted = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);}

void UGA_Sprint::OnCharacterMoved()
{
	//UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("Character Are Moved！！！！!!!!!!!!!!!! "));

	if (ActiveSprintVigorCostHandle.IsValid() || bVigorExhaustedBroadcasted) return; 
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		ActiveSprintVigorCostHandle = ASC->ApplyGameplayEffectSpecToSelf(CostSpec);
	}
}

void UGA_Sprint::OnCharacterStopped()
{
	//UE_LOG(LogAegisOdysseyAbilitySystem, Error, TEXT("Character Are Stoped！！！！!!!!!!!!!!!! "));
	if (!ActiveSprintVigorCostHandle.IsValid()) return;
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		ASC->RemoveActiveGameplayEffect(ActiveSprintVigorCostHandle, 1);
	}
	ActiveSprintVigorCostHandle.Invalidate();
}



void UGA_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                            const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (SprintSpeedEffectHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			ASC->RemoveActiveGameplayEffect(SprintSpeedEffectHandle, 1);
		}
		SprintSpeedEffectHandle.Invalidate();
	}
	if (ActiveSprintVigorCostHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			ASC->RemoveActiveGameplayEffect(ActiveSprintVigorCostHandle, 1);
		}
		ActiveSprintVigorCostHandle.Invalidate();
	}
	
	if (WaitInputReleaseTask)
	{
		WaitInputReleaseTask->EndTask();
		WaitInputReleaseTask = nullptr;
	}
	
	if (WaitVigorExhaustTask)
	{
		WaitVigorExhaustTask->EndTask();
		WaitVigorExhaustTask = nullptr;
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Sprint::InitializeVigorCost()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !SprintCostVigorClass) return;
	
	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	CostSpec = FGameplayEffectSpec(SprintCostVigorClass.GetDefaultObject(), ContextHandle , GetAbilityLevel());
	CostSpec.SetByCallerTagMagnitudes.FindOrAdd(TAG_Ability_Sprint_CostVigorValue) = SprintCost;
}

UAT_WaitVigorExhaust::UAT_WaitVigorExhaust(const FObjectInitializer& ObjectInitializer)
{
	OnWaitVigorExhaust.Clear();
	bTickingTask = true;
}

UAT_WaitVigorExhaust* UAT_WaitVigorExhaust::CreateWaitVigorExhaust(UAOGameplayAbility* OwningAbility)
{
	UAT_WaitVigorExhaust* MyObj = NewAbilityTask<UAT_WaitVigorExhaust>(OwningAbility);
	return MyObj;
}

void UAT_WaitVigorExhaust::Activate()
{
	Super::Activate();
}

void UAT_WaitVigorExhaust::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	if (AAOCharacter* SourceCharacter = Cast<AAOCharacter>(Ability->GetAvatarActorFromActorInfo()))
	{
		if (AbilitySystemComponent.IsValid())
		{
			const UAOCombatAttributeSet* AttributeSet = Cast<UAOCombatAttributeSet>(AbilitySystemComponent->GetAttributeSet(UAOCombatAttributeSet::StaticClass()));
			if (!AttributeSet) EndTask();
			if (AttributeSet->GetVigor() <= 0)
			{
				OnWaitVigorExhaust.Broadcast();
			}
			
			if (UCharacterMovementComponent* MovementComponent = SourceCharacter->GetCharacterMovement())
			{
				if (MovementComponent->Velocity.Size2D() > 1.f) OnCharacterMove.Broadcast();
				else OnCharacterStop.Broadcast();
			}
		}
	}
}

void UAT_WaitVigorExhaust::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);
}

