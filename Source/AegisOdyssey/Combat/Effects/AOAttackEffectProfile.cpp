#include "AegisOdyssey/Combat/Effects/AOAttackEffectProfile.h"

#include "AbilitySystemComponent.h"
#include "AegisOdyssey/AOAbilityTypes.h"
#include "AegisOdyssey/Equipment/AOWeaponManagerComponent.h"
#include "AegisOdyssey/Equipment/Weapons/AOWeaponInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOAttackEffectProfile)

const UAOAttackEffectProfile* FAOAttackEffectProfileRuntime::ResolveProfileFromSourceObject(const UObject* SourceObject)
{
	// 命中结算链优先吃上下文里显式带来的武器实例，避免表现配置和实际出手武器错位。
	if (const UAOWeaponInstance* WeaponInstance = Cast<UAOWeaponInstance>(SourceObject))
	{
		return WeaponInstance->GetEffectiveAttackEffectProfile();
	}

	if (const AActor* SourceActor = Cast<AActor>(SourceObject))
	{
		return ResolveProfileFromActor(SourceActor);
	}

	if (SourceObject != nullptr)
	{
		if (const AActor* OuterActor = Cast<AActor>(SourceObject->GetOuter()))
		{
			return ResolveProfileFromActor(OuterActor);
		}
	}

	return nullptr;
}

const UAOAttackEffectProfile* FAOAttackEffectProfileRuntime::ResolveProfileFromActor(const AActor* Actor)
{
	if (Actor == nullptr)
	{
		return nullptr;
	}

	// 挥舞过程里的 Begin/End 没有命中上下文，只能回到当前持有武器取默认表现 Profile。
	const APawn* OwnerPawn = Cast<APawn>(Actor);
	if (OwnerPawn == nullptr)
	{
		OwnerPawn = Cast<APawn>(Actor->GetOwner());
	}

	if (OwnerPawn == nullptr)
	{
		return nullptr;
	}

	const UAOWeaponManagerComponent* WeaponManagerComponent = OwnerPawn->FindComponentByClass<UAOWeaponManagerComponent>();
	const UAOWeaponInstance* WeaponInstance =
		WeaponManagerComponent != nullptr
			? Cast<UAOWeaponInstance>(WeaponManagerComponent->GetCurrentWeaponInstance())
			: nullptr;
	return WeaponInstance != nullptr ? WeaponInstance->GetEffectiveAttackEffectProfile() : nullptr;
}

const UAOAttackEffectProfile* FAOAttackEffectProfileRuntime::ResolveProfileFromEffectContext(
	const FAOGameplayEffectContext& EffectContext)
{
	if (const UAOAttackEffectProfile* Profile = EffectContext.GetAttackEffectProfile())
	{
		return Profile;
	}

	if (const UAOAttackEffectProfile* Profile = ResolveProfileFromSourceObject(EffectContext.GetSourceObject()))
	{
		return Profile;
	}

	if (const UAOAttackEffectProfile* Profile = ResolveProfileFromActor(EffectContext.GetEffectCauser()))
	{
		return Profile;
	}

	return ResolveProfileFromActor(EffectContext.GetOriginalInstigator());
}

void FAOAttackEffectProfileRuntime::DispatchTrigger(
	const UAOAttackEffectProfile* Profile,
	EAOAttackEffectTrigger Trigger,
	UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayCueParameters& CueParameters)
{
	if (Profile == nullptr || AbilitySystemComponent == nullptr)
	{
		return;
	}

	// 这里只做“既有战斗触发语义 -> GameplayCue 生命周期”的统一派发，不新增判定分支。
	for (const FAOAttackEffectEntry& Entry : Profile->GetEntries())
	{
		if (Entry.Trigger != Trigger)
		{
			continue;
		}

		for (const FGameplayTag& CueTag : Entry.CueTags)
		{
			if (!CueTag.IsValid())
			{
				continue;
			}

			switch (Trigger)
			{
			case EAOAttackEffectTrigger::CombatWindowBegin:
				AbilitySystemComponent->AddGameplayCue(CueTag, CueParameters);
				break;

			case EAOAttackEffectTrigger::CombatWindowEnd:
				AbilitySystemComponent->RemoveGameplayCue(CueTag);
				break;

			case EAOAttackEffectTrigger::HitConfirmed:
				AbilitySystemComponent->ExecuteGameplayCue(CueTag, CueParameters);
				break;

			default:
				break;
			}
		}
	}
}
