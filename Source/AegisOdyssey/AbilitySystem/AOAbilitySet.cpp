// Fill out your copyright notice in the Description page of Project Settings.


#include "AOAbilitySet.h"

#include "Abilities/AOGameplayAbility.h"
#include "AttributeSet.h"
#include "GameplayEffect.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOAbilitySet)


void UAOAbilitySet::GiveToAbilitySystem(UAOAbilitySystem* InAOASC, FAOAbilitySet_GrantedHandles* OutGrantedHandles,
	UObject* SourceObject) const
{
	check(InAOASC);

	// AbilitySet 的授予仍然只允许服务端执行。
	// 这一版已经撤回“运行时动态增删 AttributeSet”的方案，
	// 因此这里重新回到只负责授予 Ability / GameplayEffect 的稳定语义。
	if (!InAOASC->IsOwnerActorAuthoritative())
	{
		return;
	}

	// AbilitySet 里的 AttributeSets 本来就是授予包的一部分。
	// 这里要做的是“确保这批授予依赖的属性集在 ASC 上可用”，
	// 而不是把这层语义拆到 PawnData 的另一条旁路里。
	for (int32 SetIndex = 0; SetIndex < AttributeSets.Num(); ++SetIndex)
	{
		const FAOAbilitySet_AttributeSet& SetToGrant = AttributeSets[SetIndex];
		if (!IsValid(SetToGrant.AttributeSet))
		{
			continue;
		}

		InAOASC->EnsureSpawnedAttributeSet(SetToGrant.AttributeSet);
	}

	for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FAOAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];
		if (!IsValid(AbilityToGrant.Ability))
		{
			continue;
		}

		UAOGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UAOGameplayAbility>();

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag);

		const FGameplayAbilitySpecHandle SpecHandle = InAOASC->GiveAbility(AbilitySpec);
		if (OutGrantedHandles != nullptr)
		{
			OutGrantedHandles->AddAbilitySpecHandle(SpecHandle);
		}
	}

	for (int32 EffectIndex = 0; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
	{
		const FAOAbilitySet_GameplayEffect& EffectToGrant = GrantedGameplayEffects[EffectIndex];
		if (!IsValid(EffectToGrant.GameplayEffect))
		{
			UE_LOG(LogTemp, Error, TEXT("%s"), *GetName());
			continue;
		}

		const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle GameplayEffectHandle =
			InAOASC->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel, InAOASC->MakeEffectContext());

		if (OutGrantedHandles != nullptr)
		{
			OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
		}
	}

	for (int32 EffectIndex = 0; EffectIndex < SecondaryGameplayEffects.Num(); ++EffectIndex)
	{
		const FAOAbilitySet_SecondaryGameplayEffect& SecondaryGameplayEffect = SecondaryGameplayEffects[EffectIndex];
		if (!IsValid(SecondaryGameplayEffect.GameplayEffect))
		{
			continue;
		}

		const UGameplayEffect* GameplayEffect = SecondaryGameplayEffect.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle GameplayEffectHandle =
			InAOASC->ApplyGameplayEffectToSelf(GameplayEffect, SecondaryGameplayEffect.EffectLevel, InAOASC->MakeEffectContext());

		if (OutGrantedHandles != nullptr)
		{
			OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
		}
	}

	for (int32 EffectIndex = 0; EffectIndex < MetaGameplayEffects.Num(); ++EffectIndex)
	{
		const FAOAbilitySet_MetaGameplayEffect& MetaGameplayEffect = MetaGameplayEffects[EffectIndex];
		if (!IsValid(MetaGameplayEffect.GameplayEffect))
		{
			continue;
		}

		const UGameplayEffect* GameplayEffect = MetaGameplayEffect.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		(void)GameplayEffect;
		// Meta GE 当前仍保持原来的预留语义，不在这里直接 Apply。
	}
}

void FAOAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FAOAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

FGameplayAbilitySpecHandle FAOAbilitySet_GrantedHandles::GetPrimaryAbilitySpecHandle() const
{
	// 当前只返回这批句柄里第一条有效能力句柄，
	// 方便“单能力授予”为主的调用方拿到代表句柄。
	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			return Handle;
		}
	}

	return FGameplayAbilitySpecHandle();
}

void FAOAbilitySet_GrantedHandles::TakeFromAbilitySystem(UAOAbilitySystem* AOASC)
{
	check(AOASC);

	if (!AOASC->IsOwnerActorAuthoritative())
	{
		return;
	}

	// 这里只回收当前这批授予句柄自己记录的结果，
	// 不推断也不触碰其他来源授予出去的能力与效果。
	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			AOASC->ClearAbility(Handle);
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
	{
		if (Handle.IsValid())
		{
			AOASC->RemoveActiveGameplayEffect(Handle);
		}
	}

	AbilitySpecHandles.Reset();
	GameplayEffectHandles.Reset();
}

void FAOAbilitySet_GrantedHandles::TakeFromAbilitySystemStackAware(UAOAbilitySystem* AOASC)
{
	check(AOASC);

	if (!AOASC->IsOwnerActorAuthoritative())
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			AOASC->ClearAbility(Handle);
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
	{
		if (!Handle.IsValid())
		{
			continue;
		}

		int32 StacksToRemove = -1;

		// 可堆叠来源效果在卸下时要按层回收，
		// 避免把同类 GE 的整条活动实例直接误删。
		if (const FActiveGameplayEffect* ActiveGameplayEffect = AOASC->GetActiveGameplayEffect(Handle))
		{
			const UGameplayEffect* GameplayEffect = ActiveGameplayEffect->Spec.Def;
			if (GameplayEffect != nullptr &&
				GameplayEffect->StackingType != EGameplayEffectStackingType::None &&
				GameplayEffect->GetStackLimitCount() > 1)
			{
				StacksToRemove = ActiveGameplayEffect->Spec.GetStackCount();
			}
		}

		AOASC->RemoveActiveGameplayEffect(Handle, StacksToRemove);
	}

	AbilitySpecHandles.Reset();
	GameplayEffectHandles.Reset();
}
