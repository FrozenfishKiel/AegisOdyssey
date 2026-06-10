// Fill out your copyright notice in the Description page of Project Settings.


#include "AOAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOAttributeSet)
UAOAttributeSet::UAOAttributeSet()
{
	
}

UWorld* UAOAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

UAOAbilitySystem* UAOAttributeSet::GetAOAbilitySystemComponent() const
{
	return Cast<UAOAbilitySystem>(GetOwningAbilitySystemComponent());
}

void UAOAttributeSet::SetEffectContext(const FGameplayEffectModCallbackData& Data, FEffectProperties& Properties)
{
	Properties.EffectContextHandle = Data.EffectSpec.GetContext();
	Properties.SourceASC = Properties.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();  //返回始作俑者的ASC

	if(IsValid(Properties.SourceASC) && Properties.SourceASC->AbilityActorInfo.IsValid()&&Properties.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		Properties.SourceAvatarActor = Properties.SourceASC->AbilityActorInfo->AvatarActor.Get();//获取对象Actor
		Properties.SourceController = Properties.SourceASC->AbilityActorInfo->PlayerController.Get(); //获取对象控制器
		//如果控制器存在但是对象Actor不存在
		if(Properties.SourceController==nullptr&&Properties.SourceAvatarActor!=nullptr)
		{
			if(const APawn*Pawn = Cast<APawn>(Properties.SourceAvatarActor))
			{
				Properties.SourceController = Pawn->GetController();
			}
		}
		if(Properties.SourceController)
		{
			Properties.SourceCharacter = Cast<ACharacter>(Properties.SourceController->GetPawn());
		}
	}
	if(Data.Target.AbilityActorInfo.IsValid()&&Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Properties.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		Properties.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Properties.TargetCharacter = Cast<ACharacter>(Properties.TargetAvatarActor);  //将目标Actor转换成角色
		Properties.TargetASC = &Data.Target;
	}
}
