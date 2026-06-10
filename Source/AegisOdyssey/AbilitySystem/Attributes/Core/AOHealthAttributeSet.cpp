// Fill out your copyright notice in the Description page of Project Settings.


#include "AOHealthAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "AegisOdyssey/AOAbilityTypes.h"
#include "AegisOdyssey/AOCombatCueTags.h"
#include "AegisOdyssey/AOCombatMessageSubsystem.h"
#include "AegisOdyssey/AOCombatResultMessage.h"
#include "AegisOdyssey/AOVerbMessage.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AegisOdyssey/AOStateTags.h"
#include "AegisOdyssey/Combat/Effects/AOAttackEffectProfile.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Character/AOCharacterCombatManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHealthAttributeSet)


UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_Damage, "Gameplay.Damage");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_DamageImmunity, "Gameplay.DamageImmunity");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_DamageSelfDestruct, "Gameplay.Damage.SelfDestruct");
UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_FellOutOfWorld, "Gameplay.Damage.FellOutOfWorld");
UE_DEFINE_GAMEPLAY_TAG(TAG_AO_Damage_Message, "AegisOdyssey.Damage.Message");
UAOHealthAttributeSet::UAOHealthAttributeSet()
{
	bOutOfHealth = false;
	MaxHealthBeforeAttributeChange = 0.f;
	HealthBeforeAttributeChange = 0.f;
}

void UAOHealthAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UAOHealthAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOHealthAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOHealthAttributeSet, Damage, COND_None, REPNOTIFY_Always);

}


bool UAOHealthAttributeSet::PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}
	const bool bIsDamageExecution = Data.EvaluatedData.Attribute == GetDamageAttribute();

	// HealthAttributeSet 这一层只处理“最终进入掉血链”的结果广播。
	// 完全格挡、无敌拦截这类不实际扣血的结果，前面应该已经在更上游处理过。
	if (bIsDamageExecution && Data.EvaluatedData.Magnitude > 0.0f)
	{
		UE_LOG(
			LogAegisOdysseyCombatTrace,
			Warning,
			TEXT("[CombatTrace][HealthSet] PostExecute damage branch. Owner=%s Magnitude=%.2f CurrentDamageAttr=%.2f HealthBefore=%.2f"),
			*GetNameSafe(GetOwningActor()),
			Data.EvaluatedData.Magnitude,
			GetDamage(),
			HealthBeforeAttributeChange);

		const bool bIsDamageFromSelfDestruct = Data.EffectSpec.GetDynamicAssetTags().HasTagExact(TAG_Gameplay_DamageSelfDestruct);
		// Data.Target 指向受击方 ASC，这里只拦截正常伤害，保留自毁这类强制扣血通道。
		if (Data.Target.HasMatchingGameplayTag(TAG_Gameplay_DamageImmunity) && !bIsDamageFromSelfDestruct)
		{
			Data.EvaluatedData.Magnitude = 0.f;
			return false;
		}
	}
	
	// 先缓存修改前数值，供 PostExecute 广播差值和回调使用。
	HealthBeforeAttributeChange = GetHealth();
	MaxHealthBeforeAttributeChange = GetMaxHealth();
	
	return true;
}

void UAOHealthAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Properties;
	// 先把这次 GE 的上下文还原出来，后续伤害消息和表现派发都复用这一份事实源。
	SetEffectContext(Data, Properties);
	const bool bIsDamageFromSelfDestruct = Data.EffectSpec.GetDynamicAssetTags().HasTagExact(TAG_Gameplay_DamageSelfDestruct);
	float MininumHealth = 0.f;
	const bool bIsDamageExecution = Data.EvaluatedData.Attribute == GetDamageAttribute();
	
	FGameplayEffectContextHandle EffectContext = Data.EffectSpec.GetContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();  // 原始施加者，用于结果广播和表现上下文。
	AActor* Causer = EffectContext.GetEffectCauser();  // 实际效果来源者，可能和施加者不同。
	FAOGameplayEffectContext* SourceEffectContext = static_cast<FAOGameplayEffectContext*>(EffectContext.Get());
	check(SourceEffectContext);  // 伤害链要求统一使用 AO 自定义上下文，否则后面的结果字段无法可靠读取。

	// 扣血分支。
	if (bIsDamageExecution && Data.EvaluatedData.Magnitude > 0.0f)
	{
		FAOVerbMessage Message;
		Message.Verb = TAG_AO_Damage_Message;
		Message.Instigator = Data.EffectSpec.GetEffectContext().GetEffectCauser();
		Message.InstigatorTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
		Message.Target = GetOwningActor();
		Message.TargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
		Message.ContextTags.AppendTags(SourceEffectContext->GetDamageTypeTags());
		if (SourceEffectContext->GetAttackTag().IsValid())
		{
			Message.ContextTags.AddTag(SourceEffectContext->GetAttackTag());
		}
		if (SourceEffectContext->GetSkillTag().IsValid())
		{
			Message.ContextTags.AddTag(SourceEffectContext->GetSkillTag());
		}
		if (SourceEffectContext->GetWeaponTag().IsValid())
		{
			Message.ContextTags.AddTag(SourceEffectContext->GetWeaponTag());
		}
		const float FinalDamage = GetDamage();
		Message.Magnitude = FinalDamage;

		// 这里把最终生命伤害整理成统一战斗结果，UI / 飘字 / 命中表现都往下游读这一个结果。
		FAOCombatResultMessage CombatResultMessage;
		CombatResultMessage.ResultType = SourceEffectContext->GetWasBlocked()
			? EAOCombatResultType::Blocked
			: EAOCombatResultType::Damage;
		CombatResultMessage.FloatingTextType = SourceEffectContext->GetIsCritical()
			? EAOCombatFloatingTextType::Critical
			: EAOCombatFloatingTextType::Damage;
		CombatResultMessage.bShouldDisplayFloatingText = FinalDamage > KINDA_SMALL_NUMBER;
		CombatResultMessage.bIsCritical = SourceEffectContext->GetIsCritical();
		CombatResultMessage.bWasBlocked = SourceEffectContext->GetWasBlocked();
		CombatResultMessage.bWasParried = SourceEffectContext->GetWasParried();
		CombatResultMessage.bHitInvulnerability = SourceEffectContext->GetHitInvulnerability();
		CombatResultMessage.Instigator = Cast<AActor>(Message.Instigator.Get());
		CombatResultMessage.Target = Cast<AActor>(Message.Target.Get());
		CombatResultMessage.EffectCauser = Data.EffectSpec.GetEffectContext().GetEffectCauser();
		CombatResultMessage.CueTag = SourceEffectContext->GetWasBlocked()
			? AOCombatCueTags::GameplayCue_Combat_Block
			: AOCombatCueTags::GameplayCue_Combat_Hit;
		CombatResultMessage.AttackTag = SourceEffectContext->GetAttackTag();
		CombatResultMessage.SkillTag = SourceEffectContext->GetSkillTag();
		CombatResultMessage.WeaponTag = SourceEffectContext->GetWeaponTag();
		CombatResultMessage.DamageTypeTags = SourceEffectContext->GetDamageTypeTags();
		CombatResultMessage.HealthDamage = FinalDamage;
		if (const FHitResult* ContextHitResult = SourceEffectContext->GetHitResult())
		{
			CombatResultMessage.HitResult = *ContextHitResult;
		}

		if (UAOCombatMessageSubsystem* CombatMessageSubsystem = UAOCombatMessageSubsystem::Get(this))
		{
			CombatMessageSubsystem->BroadcastCombatResult(CombatResultMessage);
		}

		if (UAbilitySystemComponent* TargetASC = GetOwningAbilitySystemComponent())
		{
			// 受击表现同样从统一结果往下走，Cue 不再自己猜测暴击、格挡等状态。
			FGameplayCueParameters CueParameters;
			CueParameters.EffectContext = EffectContext;
			CueParameters.RawMagnitude = FinalDamage;
			CueParameters.Instigator = Instigator;
			CueParameters.EffectCauser = Causer;
			CueParameters.SourceObject = EffectContext.GetSourceObject();
			CueParameters.AggregatedSourceTags = Message.InstigatorTags;
			CueParameters.AggregatedTargetTags = Message.TargetTags;
			if (const FHitResult* ContextHitResult = SourceEffectContext->GetHitResult())
			{
				CueParameters.Location = ContextHitResult->ImpactPoint;
				CueParameters.Normal = ContextHitResult->ImpactNormal.GetSafeNormal();
				CueParameters.PhysicalMaterial = ContextHitResult->PhysMaterial.Get();
			}
			else if (AActor* TargetActor = GetOwningActor())
			{
				CueParameters.Location = TargetActor->GetActorLocation();
			}

			if (FAOGameplayEffectContext* CombatEffectContext = static_cast<FAOGameplayEffectContext*>(EffectContext.Get()))
			{
				CombatResultMessage.bTargetBroken = TargetASC->HasMatchingGameplayTag(AOStateTags::State_Combat_Broken);
				CombatEffectContext->SetTargetBroken(CombatResultMessage.bTargetBroken);
				if (CombatResultMessage.bTargetBroken)
				{
					CueParameters.AggregatedTargetTags.AddTag(AOStateTags::State_Combat_Broken);
				}
			}

			//TargetASC->ExecuteGameplayCue(CombatResultMessage.CueTag, CueParameters);
			// 命中确认后的武器攻击表现从这里统一续接，避免再额外开一套命中后表现入口。
			FAOAttackEffectProfileRuntime::DispatchTrigger(
				FAOAttackEffectProfileRuntime::ResolveProfileFromEffectContext(*SourceEffectContext),
				EAOAttackEffectTrigger::HitConfirmed,
				TargetASC,
				CueParameters);

			if (FinalDamage > KINDA_SMALL_NUMBER
				&& !SourceEffectContext->GetWasBlocked()
				&& !SourceEffectContext->GetWasParried()
				&& !SourceEffectContext->GetHitInvulnerability())
			{
				if (AAOCharacter* TargetCharacter = Cast<AAOCharacter>(GetOwningActor()))
				{
					if (UAOCharacterCombatManagerComponent* CombatManager = TargetCharacter->FindComponentByClass<UAOCharacterCombatManagerComponent>())
					{
						CombatManager->HandleConfirmedHitReact(*SourceEffectContext, FinalDamage);
					}
				}
			}
		}
		
		UE_LOG(
			LogAegisOdysseyCombatTrace,
			Warning,
			TEXT("[CombatTrace][HealthSet] Apply final health delta. Owner=%s Damage=%.2f OldHealth=%.2f NewHealth=%.2f"),
			*GetNameSafe(GetOwningActor()),
			GetDamage(),
			GetHealth(),
			FMath::Clamp(GetHealth() - GetDamage() , MininumHealth, GetMaxHealth()));

		SetHealth(FMath::Clamp(GetHealth() - GetDamage() , MininumHealth, GetMaxHealth()));
		SetDamage(0.f);
	}
	// 加血分支。
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		// Convert into +Health and then clamo
		SetHealth(FMath::Clamp(GetHealth() + GetHealing(), MininumHealth, GetMaxHealth()));
		SetHealing(0.0f);
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Clamp and fall into out of health handling below
		SetHealth(FMath::Clamp(GetHealth(), MininumHealth, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		// Notify on any requested max health changes
		OnMaxHealthChange.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxHealthBeforeAttributeChange, GetMaxHealth());
	}

	// If health has actually changed activate callbacks
	if (GetHealth() != HealthBeforeAttributeChange)
	{
		OnHealthChange.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
	}

	if ((GetHealth() <= 0.0f) && !bOutOfHealth)
	{
		OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
	}

	// Check health again in case an event above changed it.
	bOutOfHealth = (GetHealth() <= 0.0f);
}

void UAOHealthAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	if (Attribute == GetHealthAttribute())
	{
		// Do not allow health to go negative or above max health.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// Do not allow max health to drop below 1.
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void UAOHealthAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetHealthAttribute())
	{
		// Do not allow health to go negative or above max health.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// Do not allow max health to drop below 1.
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void UAOHealthAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		// Make sure current health is not greater than the new max health.
		if (GetHealth() > NewValue)
		{
			UAOAbilitySystem* LyraASC = GetAOAbilitySystemComponent();
			check(LyraASC);

			LyraASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
		}
	}

	if (bOutOfHealth && (GetHealth() > 0.0f))
	{
		bOutOfHealth = false;
	}
}


void UAOHealthAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOHealthAttributeSet, Health, OldValue);

	// Call the change callback, but without an instigator
	// This could be changed to an explicit RPC in the future
	// These events on the client should not be changing attributes

	const float CurrentHealth = GetHealth();
	const float EstimatedMagnitude = CurrentHealth - OldValue.GetCurrentValue();
	
	OnHealthChange.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);

	if (!bOutOfHealth && CurrentHealth <= 0.0f)
	{
		OnOutOfHealth.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);
	}

	bOutOfHealth = (CurrentHealth <= 0.0f);
}

void UAOHealthAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOHealthAttributeSet, MaxHealth, OldValue);
	OnMaxHealthChange.Broadcast(nullptr,nullptr,nullptr,GetMaxHealth() - OldValue.GetCurrentValue() , OldValue.GetCurrentValue(),GetMaxHealth());
}


void UAOHealthAttributeSet::OnRep_Damage(FGameplayAttributeData OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAOHealthAttributeSet, MaxHealth, OldValue);
	OnDamageChanged.Broadcast(nullptr,nullptr,nullptr,GetDamage() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(),GetDamage());
}
