// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Roll.h"
#include "NativeGameplayTags.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Controller.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_Roll)
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Roll_Cooldown, "Ability.Roll.Cooldown");

static const FGameplayTagContainer RollCooldownTags(TAG_Ability_Roll_Cooldown);

void UGA_Roll::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

const FGameplayTagContainer* UGA_Roll::GetCooldownTags() const
{
	return &RollCooldownTags;
}

bool UGA_Roll::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
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

	return true;
}

bool UGA_Roll::CheckCost(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}

	if (RollVigorCost <= 0.0f)
	{
		return true;
	}

	// 第一版翻滚体力成本直接按当前 Vigor 是否足够来判断。
	// 这样可以保证“能不能翻滚”和“翻滚后实际扣多少体力”使用同一套资源语义。
	const UAbilitySystemComponent* AbilitySystemComponent = ActorInfo != nullptr ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	const UAOCombatAttributeSet* CombatAttributeSet = AbilitySystemComponent != nullptr
		? Cast<UAOCombatAttributeSet>(AbilitySystemComponent->GetAttributeSet(UAOCombatAttributeSet::StaticClass()))
		: nullptr;
	return CombatAttributeSet != nullptr && CombatAttributeSet->GetVigor() >= RollVigorCost;
}

void UGA_Roll::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (TriggerEventData)
	{
		// 从触发事件里提取本次翻滚要用的方向、输入语义和八方向蒙太奇资源。
		for (const TSharedPtr<FGameplayAbilityTargetData>& Data : TriggerEventData->TargetData.Data)
		{
			if (Data.IsValid() && Data->GetScriptStruct() == FRollTargetData::StaticStruct())
			{
				FRollTargetData* RollTargetData = static_cast<FRollTargetData*>(Data.Get());

				InputTag = RollTargetData->InputTag;
				InputType = RollTargetData->InputType;
				PlayRate = RollTargetData->PlayRate;
				StartSection = RollTargetData->StartSection;
				StartTime = RollTargetData->StartTime;

				ForwardMontage = RollTargetData->ForwardMontage;
				RightMontage = RollTargetData->RightMontage;
				BackwardMontage = RollTargetData->BackwardMontage;
				LeftMontage = RollTargetData->LeftMontage;
				ForwardLeftMontage = RollTargetData->ForwardLeftMontage;
				ForwardRightMontage = RollTargetData->ForwardRightMontage;
				BackwardLeftMontage = RollTargetData->BackwardLeftMontage;
				BackwardRightMontage = RollTargetData->BackwardRightMontage;

				SavedExplicitDirection = RollTargetData->ExplicitDirection;
				SavedMoveInputDirection = RollTargetData->MoveInputDirection;
				break;
			}
		}
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		// 翻滚现在已经正式接入体力成本。
		// 如果 Commit 失败，就不能继续播翻滚动作，否则会出现“体力不够但动作还是出了”的假结算。
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	SelectDirectionalMontage();
	PlayMontageAnimation();
}

void UGA_Roll::ApplyCost(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

	if (RollVigorCost <= 0.0f)
	{
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = ActorInfo != nullptr ? ActorInfo->AbilitySystemComponent.Get() : nullptr)
	{
		// 这里是翻滚体力消耗的正式收口点。
		// 第一版先做固定值，保证它和格挡一样进入统一体力资源链，而不是继续遗漏在动作层外面。
		AbilitySystemComponent->ApplyModToAttribute(UAOCombatAttributeSet::GetVigorAttribute(), EGameplayModOp::Additive, -RollVigorCost);
	}
}

void UGA_Roll::PlayMontageAnimation()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	UAnimInstance* AnimInstance = ActorInfo ? ActorInfo->GetAnimInstance() : nullptr;
	UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_LightAttack::PlayMontageAnimation: AnimInstance: %s"),
		*GetNameSafe(AnimInstance));

	if (!AnimInstance)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Error, TEXT("UGA_LightAttack::PlayMontageAnimation: AnimInstance is null!"));
		return;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		FName("PlayMontageAndWait"),
		Montage,
		PlayRate,
		StartSection,
		true,
		1.0f,
		StartTime,
		false
	);

	if (MontageTask)
	{
		// 翻滚结束的所有出口都统一收束到 EndAbility，
		// 避免 Completed / BlendOut / Interrupted / Cancelled 各自走出不一样的清理路径。
		MontageTask->OnBlendOut.AddDynamic(this, &UGA_Roll::OnMontageBlendedOut);
		MontageTask->OnCompleted.AddDynamic(this, &UGA_Roll::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_Roll::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_Roll::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
}

void UGA_Roll::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Roll::OnMontageBlendedOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Roll::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Roll::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Roll::SelectDirectionalMontage()
{
	if (!CurrentActorInfo)
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(CurrentActorInfo->AvatarActor);
	if (!Pawn)
	{
		return;
	}

	AController* Controller = Pawn->GetController();
	if (!Controller)
	{
		return;
	}

	FVector MoveInputVector = SavedMoveInputDirection;

	constexpr float InputDeadZone = 0.1f;
	constexpr float DiagonalThreshold = 0.5f;

	if (!SavedExplicitDirection.IsNearlyZero())
	{
		// AI 路径不再先强制转身再前滚，而是像玩家一样做八方向判定。
		// 区别只是玩家的输入向量来自移动输入，AI 的输入向量来自外部算好的世界方向。
		MoveInputVector = SavedExplicitDirection;
		MoveInputVector.Z = 0.0f;
		MoveInputVector.Normalize();
	}
	else
	{
		// 玩家路径继续沿用现有设计：输入向量按控制器朝向解释。
		MoveInputVector.Z = 0.0f;
	}

	if (MoveInputVector.Size() > InputDeadZone)
	{
		// 所有方向判断都统一转到控制器朝向空间里解释，
		// 这样“前后左右”和“前左前右”语义才能和角色当前镜头 / 控制朝向保持一致。
		const FRotator ControlRotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		const float ForwardDot = FVector::DotProduct(MoveInputVector, ForwardDirection);
		const float RightDot = FVector::DotProduct(MoveInputVector, RightDirection);

		UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: ForwardDot=%.2f, RightDot=%.2f"),
			ForwardDot, RightDot);

		if (ForwardDot > DiagonalThreshold && RightDot > DiagonalThreshold)
		{
			Montage = ForwardRightMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Selected Forward-Right Roll"));
		}
		else if (ForwardDot > DiagonalThreshold && RightDot < -DiagonalThreshold)
		{
			Montage = ForwardLeftMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Selected Forward-Left Roll"));
		}
		else if (ForwardDot < -DiagonalThreshold && RightDot > DiagonalThreshold)
		{
			Montage = BackwardRightMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Selected Backward-Right Roll"));
		}
		else if (ForwardDot < -DiagonalThreshold && RightDot < -DiagonalThreshold)
		{
			Montage = BackwardLeftMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Selected Backward-Left Roll"));
		}
		else if (ForwardDot > DiagonalThreshold)
		{
			Montage = ForwardMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Selected Forward Roll"));
		}
		else if (ForwardDot < -DiagonalThreshold)
		{
			Montage = BackwardMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Selected Backward Roll"));
		}
		else if (RightDot > DiagonalThreshold)
		{
			Montage = RightMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Selected Right Roll"));
		}
		else if (RightDot < -DiagonalThreshold)
		{
			Montage = LeftMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Selected Left Roll"));
		}
		else
		{
			Montage = ForwardMontage;
			UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: Defaulting to Forward Roll"));
		}
	}
	else
	{
		// 没有明确输入时暂时沿用当前项目里的默认后滚方案。
		Montage = BackwardMontage;
		UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UGA_Roll::SelectDirectionalMontage: No input, using Backward Roll"));
	}

	if (!Montage)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Roll::SelectDirectionalMontage: Selected montage is null!"));
	}
}

void UGA_Roll::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (MontageTask)
	{
		// 能力结束时一定要回收 MontageTask，避免旧任务残留继续回调。
		MontageTask->EndTask();
		Montage = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Roll::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (CooldownDuration <= 0.0f)
	{
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!ASC)
	{
		return;
	}

	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (!CooldownGE)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Warning, TEXT("UGA_Block::ApplyCooldown: CooldownGameplayEffectClass is not set in blueprint!"));
		return;
	}

	FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromHandle(Handle);
	if (AbilitySpec)
	{
		AbilitySpec->SetByCallerTagMagnitudes.FindOrAdd(TAG_Ability_Roll_Cooldown) = CooldownDuration;
	}

	FGameplayEffectContextHandle EffectContext = MakeEffectContext(Handle, ActorInfo);
	float Level = GetAbilityLevel(Handle, ActorInfo);
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), Level);
	if (!SpecHandle.Data.IsValid())
	{
		return;
	}

	Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
}
