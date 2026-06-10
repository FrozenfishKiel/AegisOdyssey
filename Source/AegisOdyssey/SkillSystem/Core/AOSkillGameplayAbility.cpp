// Fill out your copyright notice in the Description page of Project Settings.

#include "AegisOdyssey/SkillSystem/Core/AOSkillGameplayAbility.h"

#include "AegisOdyssey/SkillSystem/Core/AOSkillDefinition.h"
#include "AegisOdyssey/SkillSystem/Core/AOSkillExecutionDefinition.h"
#include "AegisOdyssey/SkillSystem/Core/AOSkillInstance.h"
#include "AegisOdyssey/AbilitySystem/AOEffects/GE_Cooldown.h"
#include "AegisOdyssey/Character/CombatInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "HAL/IConsoleManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOSkillGameplayAbility)

namespace AOSkillGameplayAbilityPrivate
{
	static TAutoConsoleVariable<int32> CVarSkillDebugDraw(
		TEXT("ao.Skill.DebugDraw"),
		0,
		TEXT("Enable debug draw for AO skill execution helpers.")
	);
}

UAOSkillGameplayAbility::UAOSkillGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 技能系统默认共用冷却 GE。
	// 这样具体技能只要继承 SkillAbility，就先天然接入技能冷却主链。
	FallbackCooldownGameplayEffectClass = UGE_Cooldown::StaticClass();
}

void UAOSkillGameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	ResetHitPolicyTracking();
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAOSkillGameplayAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	ResetHitPolicyTracking();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

const FGameplayTagContainer* UAOSkillGameplayAbility::GetCooldownTags() const
{
	CachedCooldownTags.Reset();

	// SkillAbility 的冷却身份不再来自“槽位”，而是来自当前 Ability 绑定的 SkillInstance。
	if (const UAOSkillInstance* SkillInstance = Cast<UAOSkillInstance>(GetCurrentSourceObject()))
	{
		SkillInstance->GetCooldownIdentityTags(CachedCooldownTags);
	}

	return &CachedCooldownTags;
}

bool UAOSkillGameplayAbility::CheckCooldown(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	const float CooldownDuration = GetCooldownDurationSeconds(Handle, ActorInfo);
	if (CooldownDuration <= 0.0f)
	{
		// 没有配置基础冷却，等价于当前技能不接入共享冷却体系。
		return true;
	}

	FGameplayTagContainer CooldownTags;
	BuildCooldownIdentityTags(Handle, ActorInfo, CooldownTags);
	if (CooldownTags.IsEmpty())
	{
		// 没有身份标签就不强行拦，避免错误地把技能锁进一个不明确的冷却池。
		return true;
	}

	const FGameplayEffectQuery CooldownQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTags);
	if (UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get())
	{
		if (AbilitySystemComponent->GetActiveEffects(CooldownQuery).Num() > 0)
		{
			if (OptionalRelevantTags)
			{
				OptionalRelevantTags->AppendTags(CooldownTags);
			}

			return false;
		}
	}

	return true;
}

void UAOSkillGameplayAbility::ApplyCooldown(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return;
	}

	const float CooldownDuration = GetCooldownDurationSeconds(Handle, ActorInfo);
	if (CooldownDuration <= 0.0f)
	{
		return;
	}

	FGameplayTagContainer CooldownTags;
	BuildCooldownIdentityTags(Handle, ActorInfo, CooldownTags);
	if (CooldownTags.IsEmpty())
	{
		return;
	}

	const TSubclassOf<UGameplayEffect> CooldownEffectClass = ResolveCooldownGameplayEffectClass();
	if (!CooldownEffectClass)
	{
		return;
	}

	// 这里把技能身份标签直接写进冷却 GE 的动态 GrantedTags。
	// 这样共享冷却的归属就稳定落在“技能语义”上，而不是某一个输入槽位上。
	const float AbilityLevel = GetAbilityLevel(Handle, ActorInfo);
	FGameplayEffectSpecHandle CooldownSpecHandle = MakeOutgoingGameplayEffectSpec(CooldownEffectClass, AbilityLevel);
	if (!CooldownSpecHandle.IsValid() || !CooldownSpecHandle.Data.IsValid())
	{
		return;
	}

	FGameplayEffectSpec* CooldownSpec = CooldownSpecHandle.Data.Get();
	CooldownSpec->DynamicGrantedTags.AppendTags(CooldownTags);
	CooldownSpec->SetDuration(CooldownDuration, true);

	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CooldownSpecHandle);
}

UAOSkillInstance* UAOSkillGameplayAbility::GetSkillInstanceFromCurrentSourceObject() const
{
	return Cast<UAOSkillInstance>(GetCurrentSourceObject());
}

UAOSkillDefinition* UAOSkillGameplayAbility::GetSkillDefinitionFromCurrentSourceObject() const
{
	if (UAOSkillInstance* SkillInstance = GetSkillInstanceFromCurrentSourceObject())
	{
		return SkillInstance->GetSkillDefinition();
	}

	return nullptr;
}

UAOSkillExecutionDefinition* UAOSkillGameplayAbility::GetSkillExecutionDefinitionFromCurrentSourceObject() const
{
	if (const UAOSkillDefinition* SkillDefinition = GetSkillDefinitionFromCurrentSourceObject())
	{
		return SkillDefinition->GetExecutionDefinition();
	}

	return nullptr;
}

void UAOSkillGameplayAbility::RouteSkillEffectApplicationFromRuntimeActor(
	const TArray<AActor*>& Targets,
	const FHitResult* HitResult,
	AActor* EffectCauser,
	FName SegmentKey) const
{
	// 这个函数的意义是把“运行时执行体命中结果”重新收口回 SkillAbility。
	// 这样投射体、区域 Actor 不需要知道战斗尾链细节，只需要把命中列表交回来。
	ApplySkillEffectsToTargets(Targets, HitResult, EffectCauser, SegmentKey);
}

bool UAOSkillGameplayAbility::CanRuntimeActorDrawSkillDebug() const
{
	return ShouldDrawSkillDebug();
}

FName UAOSkillGameplayAbility::BuildRuntimeHitSegmentKey(FName ExplicitSegmentKey, int32 SegmentIndex) const
{
	const UAOSkillExecutionDefinition* ExecutionDefinition = GetSkillExecutionDefinitionFromCurrentSourceObject();
	const FGameplayTag AttackTag = ExecutionDefinition != nullptr
		? ExecutionDefinition->EffectConfig.AttackTag
		: FGameplayTag();

	return FAOCombatHitPolicyKeyBuilder::BuildSegmentKey(ExplicitSegmentKey, AttackTag, SegmentIndex);
}

UAOSkillInstance* UAOSkillGameplayAbility::GetSkillInstanceFromSourceObject(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	// SkillComponent 在授予 AbilitySpec 时把 SkillInstance 写进了 SourceObject。
	// 所以 SkillAbility 回到技能语义层的唯一合法入口就是这里。
	return Cast<UAOSkillInstance>(GetSourceObject(Handle, ActorInfo));
}

UAOSkillDefinition* UAOSkillGameplayAbility::ResolveSkillDefinition(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (UAOSkillInstance* SkillInstance = GetSkillInstanceFromSourceObject(Handle, ActorInfo))
	{
		return SkillInstance->GetSkillDefinition();
	}

	return nullptr;
}

UAOSkillExecutionDefinition* UAOSkillGameplayAbility::ResolveSkillExecutionDefinition(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (const UAOSkillDefinition* SkillDefinition = ResolveSkillDefinition(Handle, ActorInfo))
	{
		return SkillDefinition->GetExecutionDefinition();
	}

	return nullptr;
}

void UAOSkillGameplayAbility::BuildCooldownIdentityTags(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer& OutTags) const
{
	OutTags.Reset();

	if (const UAOSkillInstance* SkillInstance = GetSkillInstanceFromSourceObject(Handle, ActorInfo))
	{
		SkillInstance->GetCooldownIdentityTags(OutTags);
	}
}

float UAOSkillGameplayAbility::GetCooldownDurationSeconds(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (const UAOSkillInstance* SkillInstance = GetSkillInstanceFromSourceObject(Handle, ActorInfo))
	{
		return SkillInstance->GetConfiguredCooldownDuration();
	}

	return 0.0f;
}

TSubclassOf<UGameplayEffect> UAOSkillGameplayAbility::ResolveCooldownGameplayEffectClass() const
{
	if (UGameplayEffect* CooldownGE = GetCooldownGameplayEffect())
	{
		return CooldownGE->GetClass();
	}

	return FallbackCooldownGameplayEffectClass;
}

void UAOSkillGameplayAbility::ApplySkillEffectsToTargets(
	const TArray<AActor*>& Targets,
	const FHitResult* HitResult,
	AActor* EffectCauser,
	FName SegmentKey) const
{
	if (Targets.IsEmpty())
	{
		return;
	}

	AActor* SourceActor = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceActor || !SourceASC)
	{
		return;
	}

	const UAOSkillExecutionDefinition* ExecutionDefinition = GetSkillExecutionDefinitionFromCurrentSourceObject();
	const FAOCombatHitPolicy HitPolicy = ExecutionDefinition != nullptr
		? ExecutionDefinition->EffectConfig.HitPolicy
		: FAOCombatHitPolicy();
	const double CurrentTimeSeconds = GetWorld() != nullptr ? GetWorld()->GetTimeSeconds() : 0.0;
	const FName ResolvedSegmentKey =
		HitPolicy.PolicyType == EAOCombatHitPolicyType::SegmentedHit
			? BuildRuntimeHitSegmentKey(SegmentKey)
			: SegmentKey;

	TArray<TSubclassOf<UGameplayEffect>> MetaEffects;
	GetSkillMetaGameplayEffects(MetaEffects);
	if (MetaEffects.IsEmpty())
	{
		// 没有配置效果就不往后推，说明当前技能只完成了命中采集，但没有定义结算。
		return;
	}

	// 到这里为止，技能执行层的职责就结束了：
	// 它只需要给出完成的“命中目标列表”，后续结算继续复用既有战斗尾链。
	for (AActor* Target : Targets)
	{
		if (!Target || !Target->Implements<UCombatInterface>())
		{
			continue;
		}

		if (!HitTracker.TryMarkHit(Target, HitPolicy, ResolvedSegmentKey, CurrentTimeSeconds))
		{
			continue;
		}

		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Target))
		{
			FAttackedInfo AttackedInfo;
			AttackedInfo.SourceASC = SourceASC;
			AttackedInfo.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
			AttackedInfo.SourceObject = GetCurrentSourceObject();
			AttackedInfo.EffectCauser = EffectCauser != nullptr ? EffectCauser : SourceActor;
			AttackedInfo.MetaGameplayEffects = MetaEffects;
			if (HitResult != nullptr)
			{
				AttackedInfo.HitResult = *HitResult;
			}

			if (const UAOSkillDefinition* SkillDefinition = GetSkillDefinitionFromCurrentSourceObject())
			{
				AttackedInfo.SkillTag = SkillDefinition->PrimarySkillTag;
				if (!AttackedInfo.SkillTag.IsValid())
				{
					for (const FGameplayTag& SkillTag : SkillDefinition->SkillTags)
					{
						if (SkillTag.IsValid())
						{
							AttackedInfo.SkillTag = SkillTag;
							break;
						}
					}
				}
			}

			if (ExecutionDefinition != nullptr)
			{
				AttackedInfo.AttackTag = ExecutionDefinition->EffectConfig.AttackTag;
				AttackedInfo.DamageTypeTags = ExecutionDefinition->EffectConfig.DamageTypeTags;
			}

			CombatInterface->ApplyDamageToTarget(AttackedInfo);
		}
	}
}

void UAOSkillGameplayAbility::ResetHitPolicyTracking() const
{
	HitTracker.Reset();
}

bool UAOSkillGameplayAbility::ShouldDrawSkillDebug() const
{
	const UAOSkillExecutionDefinition* ExecutionDefinition = GetSkillExecutionDefinitionFromCurrentSourceObject();

	return ExecutionDefinition
		&& ExecutionDefinition->DebugConfig.bEnableDebugDraw
		&& AOSkillGameplayAbilityPrivate::CVarSkillDebugDraw.GetValueOnGameThread() != 0;
}

void UAOSkillGameplayAbility::GetSkillMetaGameplayEffects(TArray<TSubclassOf<UGameplayEffect>>& OutEffects) const
{
	OutEffects.Reset();

	if (const UAOSkillExecutionDefinition* ExecutionDefinition = GetSkillExecutionDefinitionFromCurrentSourceObject())
	{
		OutEffects.Append(ExecutionDefinition->EffectConfig.MetaGameplayEffects);
	}
}

void UAOSkillGameplayAbility::ExecuteSkillCue(const FGameplayTag& CueTag, const FGameplayCueParameters& CueParameters) const
{
	if (!CueTag.IsValid())
	{
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
	{
		AbilitySystemComponent->ExecuteGameplayCue(CueTag, CueParameters);
	}
}

FGameplayCueParameters UAOSkillGameplayAbility::BuildSkillCueParameters(
	const FVector& Location,
	const FVector& Normal,
	AActor* EffectCauser) const
{
	FGameplayCueParameters CueParameters;
	CueParameters.Location = Location;
	CueParameters.Normal = Normal.GetSafeNormal();
	CueParameters.AbilityLevel = GetAbilityLevel();
	CueParameters.EffectCauser = EffectCauser != nullptr ? EffectCauser : GetAvatarActorFromActorInfo();
	CueParameters.Instigator = GetAvatarActorFromActorInfo();
	CueParameters.SourceObject = GetCurrentSourceObject();
	return CueParameters;
}
