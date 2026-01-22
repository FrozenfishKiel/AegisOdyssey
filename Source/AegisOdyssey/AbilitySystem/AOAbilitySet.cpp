// Fill out your copyright notice in the Description page of Project Settings.


#include "AOAbilitySet.h"

#include "Abilities/AOGameplayAbility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOAbilitySet)


void UAOAbilitySet::GiveToAbilitySystem(UAOAbilitySystem* InAOASC, FAOAbilitySet_GrantedHandles* OutGrantedHandles,
										UObject* SourceObject) const
{
	check(InAOASC);

	if (!InAOASC->IsOwnerActorAuthoritative()) return; //确保拥有服务器权限

	for (int32 SetIndex = 0 ; SetIndex < AttributeSets.Num() ; SetIndex++)
	{
		const FAOAbilitySet_AttributeSet& SetToGrant = AttributeSets[SetIndex];

		if (!IsValid(SetToGrant.AttributeSet))
		{
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(InAOASC->GetOwner(), SetToGrant.AttributeSet);
		InAOASC->AddAttributeSetSubobject(NewSet);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAttributeSet(NewSet);
		}
	}
	
	for (int32 AbilityIndex = 0 ; AbilityIndex < GrantedGameplayAbilities.Num() ; AbilityIndex++)
	{
		const FAOAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

		if (!IsValid(AbilityToGrant.Ability))
		{
			continue;
		}

		UAOGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UAOGameplayAbility>();

		FGameplayAbilitySpec AbilitySpec(AbilityCDO,AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag);

		const FGameplayAbilitySpecHandle Spec = InAOASC->GiveAbility(AbilitySpec);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(Spec);
		}
	}

	for (int32 EffectIndex = 0 ; EffectIndex < GrantedGameplayEffects.Num() ; EffectIndex++)
	{
		const FAOAbilitySet_GameplayEffect& EffectToGrant = GrantedGameplayEffects[EffectIndex];

		if (!IsValid(EffectToGrant.GameplayEffect))
		{
			UE_LOG(LogTemp, Error, TEXT("%s"), *GetName());
			continue;
		}

		const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle GameplayEffectHandle = InAOASC->ApplyGameplayEffectToSelf(GameplayEffect,EffectToGrant.EffectLevel,InAOASC->MakeEffectContext());

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
		}
	}

	for (int32 EffectIndex = 0 ; EffectIndex < SecondaryGameplayEffects.Num() ; EffectIndex++)
	{
		const FAOAbilitySet_SecondaryGameplayEffect& SecondaryGameplayEffect = SecondaryGameplayEffects[EffectIndex];

		if (!IsValid(SecondaryGameplayEffect.GameplayEffect)) continue;

		const UGameplayEffect* GameplayEffect = SecondaryGameplayEffect.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle GameplayEffectHandle = InAOASC->ApplyGameplayEffectToSelf(GameplayEffect,SecondaryGameplayEffect.EffectLevel,InAOASC->MakeEffectContext());

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
		}
	}
	
	for (int32 EffectIndex = 0 ; EffectIndex < MetaGameplayEffects.Num() ; EffectIndex++)
	{
		const FAOAbilitySet_MetaGameplayEffect& MetaGameplayEffect = MetaGameplayEffects[EffectIndex];

		if (!IsValid(MetaGameplayEffect.GameplayEffect)) continue;

		const UGameplayEffect* GameplayEffect = MetaGameplayEffect.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle GameplayEffectHandle = InAOASC->ApplyGameplayEffectToSelf(GameplayEffect,MetaGameplayEffect.EffectLevel,InAOASC->MakeEffectContext());

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
		}
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

void FAOAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* Set)
{
	GrantedAttributeSets.Add(Set);	
}

//移除GAS相关
void FAOAbilitySet_GrantedHandles::TakeFromAbilitySystem(UAOAbilitySystem* AOASC)
{
	check(AOASC);

	if (!AOASC->IsOwnerActorAuthoritative()) return;

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			AOASC->ClearAbility(Handle); //移除所有的技能
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
	{
		if (Handle.IsValid())
		{
			AOASC->RemoveActiveGameplayEffect(Handle);  //移除所有已经激活的游戏效果
		}
	}

	for (UAttributeSet* Set : GrantedAttributeSets)
	{
		AOASC->RemoveSpawnedAttribute(Set); //移除所有生成的AttributeSet
		
	}

	//清空数组成员但保留容量大小
	AbilitySpecHandles.Reset();
	GameplayEffectHandles.Reset();
	GrantedAttributeSets.Reset();
}

