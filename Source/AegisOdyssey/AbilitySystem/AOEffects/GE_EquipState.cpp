// Fill out your copyright notice in the Description page of Project Settings.


#include "GE_EquipState.h"

#include "GameplayEffectComponent.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GE_EquipState)

UGE_EquipState::UGE_EquipState(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
}

void UCharacterWeaponStateOption::SetDynamicGrantedTags(AActor* TargetActor, const FGameplayTag& GrantedTags)
{
	UAOExtPawnComponent* ExtPawn = UAOExtPawnComponent::FindAOExtPawnComponent(TargetActor);
	UAbilitySystemComponent* SourceASC = ExtPawn->GetAbilitySystemComponent();
	if (!ExtPawn || !SourceASC) return;
	const FActiveGameplayEffectsContainer& ActiveGameplayEffect = SourceASC->GetActiveGameplayEffects();  //获取当前对象的所有游戏效果
	FGameplayEffectQuery EffectQuery;  //游戏效果查找器

	for (auto& ActiveEfffect : ActiveGameplayEffect.GetActiveEffects(EffectQuery))
	{
		const FActiveGameplayEffect* RealActiveEffect = SourceASC->GetActiveGameplayEffect(ActiveEfffect);
		if (RealActiveEffect->Spec.Def->IsA(UGE_EquipState::StaticClass()))
		{
			UClass* EffectClass = RealActiveEffect->Spec.Def->GetClass();
			SourceASC->RemoveActiveEffects(EffectQuery);  //先移除效果再添加

			FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();  //创建上下文
			EffectContextHandle.AddSourceObject(TargetActor);
			FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(UGE_EquipState::StaticClass(),1.0,EffectContextHandle);
			UGE_EquipState* Effect = EffectClass->GetDefaultObject<UGE_EquipState>();
			
			UTargetTagsGameplayEffectComponent& TagsGameplayEffectComponent =  Effect->FindOrAddComponent<UTargetTagsGameplayEffectComponent>();  //创建组件
			
			FInheritedTagContainer InheritedTagContainer;
			InheritedTagContainer.AddTag(GrantedTags);

			TagsGameplayEffectComponent.SetAndApplyTargetTagChanges(InheritedTagContainer);  //更新标签

			SourceASC->ApplyGameplayEffectToSelf(Effect,1.0,EffectContextHandle);

			// 这个函数功能等价于刷新玩家身上效果（代码手动）
		}
	}
}
