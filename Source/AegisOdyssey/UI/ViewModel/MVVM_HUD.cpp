// Fill out your copyright notice in the Description page of Project Settings.

#include "MVVM_HUD.h"

#include "AegisOdyssey/AOCombatResultMessage.h"
#include "AegisOdyssey/AOStateTags.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Combat/AOCombatAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOHealthAttributeSet.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/Player/AOPlayerController.h"
#include "AegisOdyssey/Player/AOPlayerState.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_CombatFeedbackFeed.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_CombatResources.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_Crafting.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_ItemHoverTooltip.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_LocalCombatState.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_TargetHealthBarCollection.h"
#include "AegisOdyssey/UI/ViewModel/AIDebug/MVVM_AIDecisionDebug.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_HUD)

namespace MVVMHUDPrivate
{
	constexpr int32 MaxPendingInventoryAcquisitionEntries = 16;

	struct FHUDAttributeBindingDefinition
	{
		FGameplayAttribute Attribute;
		TFunction<void(UMVVM_HUD&, float)> ApplyValue;
	};
}

UMVVM_HUD::UMVVM_HUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMVVM_HUD::InitializeChildViewModels()
{
	EnsureChildViewModels();
}

void UMVVM_HUD::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.RepNotifyCondition = REPNOTIFY_Always;

	DOREPLIFETIME_WITH_PARAMS(ThisClass, Health, Params);
	DOREPLIFETIME_WITH_PARAMS(ThisClass, MaxHealth, Params);
	DOREPLIFETIME_WITH_PARAMS(ThisClass, Vigor, Params);
	DOREPLIFETIME_WITH_PARAMS(ThisClass, MaxVigor, Params);
	DOREPLIFETIME_WITH_PARAMS(ThisClass, Stamina, Params);
	DOREPLIFETIME_WITH_PARAMS(ThisClass, MaxStamina, Params);
}

void UMVVM_HUD::SetPlayerViewModelParams(const FPlayerMainHUDViewModelParams& Params)
{
	EnsureChildViewModels();

	// 这里只做“非空覆盖”，允许外部分阶段把依赖一点点补齐，
	// 不会因为后续只更新其中一个引用就把其他已存在的观察源清空。
	if (Params.ASC != nullptr)
	{
		PlayerViewModelParams.ASC = Params.ASC;
	}

	if (Params.PC != nullptr)
	{
		PlayerViewModelParams.PC = Params.PC;
	}

	if (Params.PS != nullptr)
	{
		PlayerViewModelParams.PS = Params.PS;
	}

	if (Params.SkillComponent != nullptr)
	{
		PlayerViewModelParams.SkillComponent = Params.SkillComponent;
	}

	if (TargetHealthBarCollectionViewModel != nullptr)
	{
		TargetHealthBarCollectionViewModel->SetObserverComponent(
			GetSourcePC() != nullptr ? GetSourcePC()->GetLocalTargetHealthBarObserverComponent() : nullptr);
	}

	OnParamSet();
}

void UMVVM_HUD::OnParamSet_Implementation()
{
	EnsureChildViewModels();

	UAOAbilitySystem* SourceASC = GetSourceASC();
	if (SourceASC == nullptr)
	{
		UnbindAttributeDelegates();
		RefreshLocalCombatStateFromSource();
		RefreshSkillObservationData();
		return;
	}

	const UAOHealthAttributeSet* HealthAttributeSet = SourceASC->GetSet<UAOHealthAttributeSet>();
	if (!HealthAttributeSet)
	{
		UnbindAttributeDelegates();
		RefreshLocalCombatStateFromSource();
		RefreshSkillObservationData();
		return;
	}

	const UAOCombatAttributeSet* CombatAttributeSet = SourceASC->GetSet<UAOCombatAttributeSet>();
	if (!CombatAttributeSet)
	{
		UnbindAttributeDelegates();
		RefreshLocalCombatStateFromSource();
		RefreshSkillObservationData();
		return;
	}

	BindAttributeDelegates();

	SetMaxHealth(HealthAttributeSet->GetMaxHealth());
	SetHealth(HealthAttributeSet->GetHealth());
	SetMaxVigor(CombatAttributeSet->GetMaxVigor());
	SetVigor(CombatAttributeSet->GetVigor());
	SetMaxStamina(CombatAttributeSet->GetMaxStamina());
	SetStamina(CombatAttributeSet->GetStamina());
	RefreshLocalCombatStateFromSource();

	// HUD 数值观察源接好以后，再刷新技能观察数据。
	RefreshSkillObservationData();
}

UAOAbilitySystem* UMVVM_HUD::GetSourceASC() const
{
	return Cast<UAOAbilitySystem>(PlayerViewModelParams.ASC);
}

AAOPlayerController* UMVVM_HUD::GetSourcePC() const
{
	return Cast<AAOPlayerController>(PlayerViewModelParams.PC);
}

AAOPlayerState* UMVVM_HUD::GetSourcePS() const
{
	return Cast<AAOPlayerState>(PlayerViewModelParams.PS);
}

UAOSkillComponent* UMVVM_HUD::GetSourceSkillComponent() const
{
	return PlayerViewModelParams.SkillComponent.Get();
}

void UMVVM_HUD::SetHealth(float InHealth)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(Health, InHealth))
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Health, this);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	}

	if (CombatResourcesViewModel != nullptr)
	{
		CombatResourcesViewModel->SetHealth(InHealth);
	}
}

void UMVVM_HUD::SetMaxHealth(float InMaxHealth)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MaxHealth, InMaxHealth))
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MaxHealth, this);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	}

	if (CombatResourcesViewModel != nullptr)
	{
		CombatResourcesViewModel->SetMaxHealth(InMaxHealth);
	}
}

float UMVVM_HUD::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? Health / MaxHealth : 0.0f;
}

void UMVVM_HUD::SetMaxVigor(float InMaxVigor)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MaxVigor, InMaxVigor))
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MaxVigor, this);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetVigorPercent);
	}

	if (CombatResourcesViewModel != nullptr)
	{
		CombatResourcesViewModel->SetMaxVigor(InMaxVigor);
	}
}

float UMVVM_HUD::GetVigorPercent() const
{
	return MaxVigor > 0.0f ? Vigor / MaxVigor : 0.0f;
}

void UMVVM_HUD::SetVigor(float InVigor)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(Vigor, InVigor))
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Vigor, this);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetVigorPercent);
	}

	if (CombatResourcesViewModel != nullptr)
	{
		CombatResourcesViewModel->SetVigor(InVigor);
	}
}

void UMVVM_HUD::SetStamina(float InStamina)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(Stamina, InStamina))
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Stamina, this);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetStaminaPercent);
	}

	if (CombatResourcesViewModel != nullptr)
	{
		CombatResourcesViewModel->SetStamina(InStamina);
	}
}

float UMVVM_HUD::GetStaminaPercent() const
{
	return MaxStamina > 0.0f ? Stamina / MaxStamina : 0.0f;
}

void UMVVM_HUD::SetMaxStamina(float InMaxStamina)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MaxStamina, InMaxStamina))
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MaxStamina, this);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetStaminaPercent);
	}

	if (CombatResourcesViewModel != nullptr)
	{
		CombatResourcesViewModel->SetMaxStamina(InMaxStamina);
	}
}

void UMVVM_HUD::SetInteractionOptions(const TArray<FInteractionOption>& InInteractionOptions)
{
	InteractionOptions = InInteractionOptions;
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetInteractionOptions);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetPrimaryInteractionOption);
}

FInteractionOption UMVVM_HUD::GetPrimaryInteractionOption() const
{
	return InteractionOptions.Num() > 0 ? InteractionOptions[0] : FInteractionOption();
}

void UMVVM_HUD::RefreshSkillObservationData()
{
	if (UAOSkillComponent* SkillComponent = GetSourceSkillComponent())
	{
		SetSkillObservationData(SkillComponent->GetSkillSlotViewDataList(), SkillComponent->GetEquippedSkillViewDataList());
		return;
	}

	SetSkillObservationData(TArray<FAOSkillSlotViewData>(), TArray<FAOEquippedSkillViewData>());
}

void UMVVM_HUD::SetSkillObservationData(
	const TArray<FAOSkillSlotViewData>& InSkillSlotViewDataList,
	const TArray<FAOEquippedSkillViewData>& InEquippedSkillViewDataList)
{
	// 这里不做复杂比较，直接把技能观察快照作为 HUD 数据面重新发布。
	SkillSlotViewDataList = InSkillSlotViewDataList;
	EquippedSkillViewDataList = InEquippedSkillViewDataList;

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetSkillSlotViewDataList);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetEquippedSkillViewDataList);
	OnSkillObservationDataChanged.Broadcast();
}

bool UMVVM_HUD::GetSkillSlotCooldownState(int32 SlotIndex, float& OutTimeRemaining, float& OutTotalDuration) const
{
	OutTimeRemaining = 0.0f;
	OutTotalDuration = 0.0f;

	if (const UAOSkillComponent* SkillComponent = GetSourceSkillComponent())
	{
		return SkillComponent->GetSkillSlotCooldownState(SlotIndex, OutTimeRemaining, OutTotalDuration);
	}

	return false;
}

float UMVVM_HUD::GetSkillSlotCooldownRemaining(int32 SlotIndex) const
{
	float TimeRemaining = 0.0f;
	float TotalDuration = 0.0f;
	GetSkillSlotCooldownState(SlotIndex, TimeRemaining, TotalDuration);
	return TimeRemaining;
}

bool UMVVM_HUD::IsSkillSlotOnCooldown(int32 SlotIndex) const
{
	float TimeRemaining = 0.0f;
	float TotalDuration = 0.0f;
	return GetSkillSlotCooldownState(SlotIndex, TimeRemaining, TotalDuration);
}

void UMVVM_HUD::ApplyCombatFeedbackViewData(const FAOCombatFeedbackViewData& FeedbackViewData)
{

	FAOCombatFeedbackViewData ResolvedFeedback = FeedbackViewData;
	if (ResolvedFeedback.SequenceId <= 0)
	{
		const float EventWorldTimeSeconds = GetWorld() != nullptr ? GetWorld()->GetTimeSeconds() : 0.0f;
		++CombatFeedbackSequence;
		ResolvedFeedback.SequenceId = CombatFeedbackSequence;
		ResolvedFeedback.EventWorldTimeSeconds = EventWorldTimeSeconds;
	}

	if (CombatFeedbackFeedViewModel != nullptr)
	{
		CombatFeedbackFeedViewModel->ApplyCombatFeedbackViewData(ResolvedFeedback);
	}

	if (LocalCombatStateViewModel != nullptr && ResolvedFeedback.bIsLocalRelevant)
	{
		LocalCombatStateViewModel->SetLastResultType(ResolvedFeedback.ResultType);
		LocalCombatStateViewModel->SetBroken(ResolvedFeedback.bIsLocalTarget && ResolvedFeedback.bTargetBroken);
		LocalCombatStateViewModel->SetParried(ResolvedFeedback.bIsLocalTarget && ResolvedFeedback.bWasParried);
	}

	RefreshLocalCombatStateFromSource();
}

void UMVVM_HUD::ApplyInventoryAcquisitionNotification(const FAOInventoryAcquisitionNotification& Notification)
{
	LatestInventoryAcquisition = Notification;
	if (LatestInventoryAcquisition.SequenceId <= 0)
	{
		const float EventWorldTimeSeconds = GetWorld() != nullptr ? GetWorld()->GetTimeSeconds() : 0.0f;
		++InventoryAcquisitionSequence;
		LatestInventoryAcquisition.SequenceId = InventoryAcquisitionSequence;
		LatestInventoryAcquisition.EventWorldTimeSeconds = EventWorldTimeSeconds;
	}

	PendingInventoryAcquisitionList.Add(LatestInventoryAcquisition);
	if (PendingInventoryAcquisitionList.Num() > MVVMHUDPrivate::MaxPendingInventoryAcquisitionEntries)
	{
		PendingInventoryAcquisitionList.RemoveAt(0, PendingInventoryAcquisitionList.Num() - MVVMHUDPrivate::MaxPendingInventoryAcquisitionEntries);
	}

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetLatestInventoryAcquisition);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetPendingInventoryAcquisitionList);
	OnInventoryAcquisitionChanged.Broadcast();
	OnInventoryAcquisitionReceived.Broadcast(LatestInventoryAcquisition);
}

TArray<FAOInventoryAcquisitionNotification> UMVVM_HUD::ConsumePendingInventoryAcquisitionList()
{
	TArray<FAOInventoryAcquisitionNotification> Result = PendingInventoryAcquisitionList;
	ClearPendingInventoryAcquisitionList();
	return Result;
}

void UMVVM_HUD::ClearPendingInventoryAcquisitionList()
{
	if (PendingInventoryAcquisitionList.IsEmpty())
	{
		return;
	}

	PendingInventoryAcquisitionList.Reset();
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetPendingInventoryAcquisitionList);
}

void UMVVM_HUD::OnRep_Health()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	UE_LOG(LogAegisOdysseyPlayer, Warning, TEXT("客户端获得 Health"));
}

void UMVVM_HUD::OnRep_MaxHealth()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	UE_LOG(LogAegisOdysseyPlayer, Warning, TEXT("客户端获得 MaxHealth"));
}

void UMVVM_HUD::OnRep_MaxVigor()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetVigorPercent);
	UE_LOG(LogAegisOdysseyPlayer, Warning, TEXT("客户端获得 MaxVigor"));
}

void UMVVM_HUD::OnRep_Vigor()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetVigorPercent);
	UE_LOG(LogAegisOdysseyPlayer, Warning, TEXT("客户端获得 Vigor"));
}

void UMVVM_HUD::OnRep_Stamina()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetStaminaPercent);
}

void UMVVM_HUD::OnRep_MaxStamina()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetStaminaPercent);
}

void UMVVM_HUD::EnsureChildViewModels()
{
	if (CombatResourcesViewModel == nullptr)
	{
		CombatResourcesViewModel = NewObject<UMVVM_CombatResources>(this);
	}

	if (LocalCombatStateViewModel == nullptr)
	{
		LocalCombatStateViewModel = NewObject<UMVVM_LocalCombatState>(this);
	}

	if (CombatFeedbackFeedViewModel == nullptr)
	{
		CombatFeedbackFeedViewModel = NewObject<UMVVM_CombatFeedbackFeed>(this);
	}

	if (TargetHealthBarCollectionViewModel == nullptr)
	{
		TargetHealthBarCollectionViewModel = NewObject<UMVVM_TargetHealthBarCollection>(this);
	}

	if (CraftingViewModel == nullptr)
	{
		CraftingViewModel = NewObject<UMVVM_Crafting>(this);
	}

	if (ItemHoverTooltipViewModel == nullptr)
	{
		ItemHoverTooltipViewModel = NewObject<UMVVM_ItemHoverTooltip>(this);
	}

	if (AIDecisionDebugViewModel == nullptr)
	{
		AIDecisionDebugViewModel = NewObject<UMVVM_AIDecisionDebug>(this);
	}
}

void UMVVM_HUD::BindAttributeDelegates()
{
	UAOAbilitySystem* SourceASC = GetSourceASC();
	if (SourceASC == nullptr)
	{
		UnbindAttributeDelegates();
		return;
	}

	if (BoundAttributeASC.Get() == SourceASC && !ActiveAttributeBindings.IsEmpty())
	{
		return;
	}

	UnbindAttributeDelegates();

	const UAOHealthAttributeSet* HealthAttributeSet = SourceASC->GetSet<UAOHealthAttributeSet>();
	const UAOCombatAttributeSet* CombatAttributeSet = SourceASC->GetSet<UAOCombatAttributeSet>();
	if (HealthAttributeSet == nullptr || CombatAttributeSet == nullptr)
	{
		return;
	}

	// 这里把“哪些属性要进 HUD”收束成单一声明表。
	// 后续新增 HUD 数值时，只需要在这里补一条 Attribute -> ApplyValue 映射，
	// 不需要再改头文件成员、重复写绑定/解绑样板。
	const TArray<MVVMHUDPrivate::FHUDAttributeBindingDefinition> BindingDefinitions =
	{
		{
			HealthAttributeSet->GetMaxHealthAttribute(),
			[](UMVVM_HUD& ViewModel, float NewValue)
			{
				ViewModel.SetMaxHealth(NewValue);
			}
		},
		{
			HealthAttributeSet->GetHealthAttribute(),
			[](UMVVM_HUD& ViewModel, float NewValue)
			{
				ViewModel.SetHealth(NewValue);
			}
		},
		{
			CombatAttributeSet->GetMaxVigorAttribute(),
			[](UMVVM_HUD& ViewModel, float NewValue)
			{
				ViewModel.SetMaxVigor(NewValue);
			}
		},
		{
			CombatAttributeSet->GetVigorAttribute(),
			[](UMVVM_HUD& ViewModel, float NewValue)
			{
				ViewModel.SetVigor(NewValue);
			}
		},
		{
			CombatAttributeSet->GetMaxStaminaAttribute(),
			[](UMVVM_HUD& ViewModel, float NewValue)
			{
				ViewModel.SetMaxStamina(NewValue);
			}
		},
		{
			CombatAttributeSet->GetStaminaAttribute(),
			[](UMVVM_HUD& ViewModel, float NewValue)
			{
				ViewModel.SetStamina(NewValue);
			}
		}
	};

	ActiveAttributeBindings.Reserve(BindingDefinitions.Num());
	for (const MVVMHUDPrivate::FHUDAttributeBindingDefinition& BindingDefinition : BindingDefinitions)
	{
		RegisterAttributeDelegate(
			SourceASC,
			BindingDefinition.Attribute,
			[this, ApplyValue = BindingDefinition.ApplyValue](float NewValue)
			{
				ApplyValue(*this, NewValue);
			});
	}

	BoundAttributeASC = SourceASC;
}

void UMVVM_HUD::UnbindAttributeDelegates()
{
	UAbilitySystemComponent* SourceASC = BoundAttributeASC.Get();
	if (SourceASC == nullptr)
	{
		ActiveAttributeBindings.Reset();
		BoundAttributeASC.Reset();
		return;
	}

	for (const FHUDAttributeDelegateBinding& ActiveBinding : ActiveAttributeBindings)
	{
		if (ActiveBinding.DelegateHandle.IsValid() && ActiveBinding.Attribute.IsValid())
		{
			SourceASC->GetGameplayAttributeValueChangeDelegate(ActiveBinding.Attribute).Remove(ActiveBinding.DelegateHandle);
		}
	}

	ActiveAttributeBindings.Reset();
	BoundAttributeASC.Reset();
}

void UMVVM_HUD::RegisterAttributeDelegate(
	UAbilitySystemComponent* SourceASC,
	const FGameplayAttribute& Attribute,
	const TFunction<void(float)>& ApplyValue)
{
	if (SourceASC == nullptr || !Attribute.IsValid() || !ApplyValue)
	{
		return;
	}

	FHUDAttributeDelegateBinding BindingRecord;
	BindingRecord.Attribute = Attribute;
	BindingRecord.DelegateHandle = SourceASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddLambda(
		[ApplyValue](const FOnAttributeChangeData& Data)
		{
			ApplyValue(Data.NewValue);
		});

	ActiveAttributeBindings.Add(MoveTemp(BindingRecord));
}

void UMVVM_HUD::RefreshLocalCombatStateFromSource()
{
	if (LocalCombatStateViewModel == nullptr)
	{
		return;
	}

	const UAbilitySystemComponent* AbilitySystemComponent = GetSourceASC();
	if (AbilitySystemComponent == nullptr)
	{
		LocalCombatStateViewModel->SetInCombat(false);
		LocalCombatStateViewModel->SetBlocking(false);
		LocalCombatStateViewModel->SetBroken(false);
		LocalCombatStateViewModel->SetParried(false);
		LocalCombatStateViewModel->SetAbilityInputBlocked(false);
		return;
	}

	FGameplayTagContainer OwnedTags;
	AbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);

	const bool bInCombat =
		OwnedTags.HasTag(AOStateTags::State_Combat_Engaging)
		|| OwnedTags.HasTag(AOStateTags::State_Combat_Combating)
		|| OwnedTags.HasTag(AOStateTags::State_Combat_Preparation)
		|| OwnedTags.HasTag(AOStateTags::State_Combat_Recovery)
		|| OwnedTags.HasTag(AOStateTags::State_Combat_Block)
		|| OwnedTags.HasTag(AOStateTags::State_Combat_Parried)
		|| OwnedTags.HasTag(AOStateTags::State_Combat_Broken);

	LocalCombatStateViewModel->SetInCombat(bInCombat);
	LocalCombatStateViewModel->SetBlocking(OwnedTags.HasTag(AOStateTags::State_Combat_Block));
	LocalCombatStateViewModel->SetBroken(OwnedTags.HasTag(AOStateTags::State_Combat_Broken));
	LocalCombatStateViewModel->SetParried(OwnedTags.HasTag(AOStateTags::State_Combat_Parried));
	LocalCombatStateViewModel->SetAbilityInputBlocked(
		AbilitySystemComponent->HasMatchingGameplayTag(TAG_Gameplay_AbilityInputBlocked));
}
