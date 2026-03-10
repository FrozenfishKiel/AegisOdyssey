// Fill out your copyright notice in the Description page of Project Settings.


#include "AOCombatAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "AegisOdyssey/AOLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCombatAttributeSet)

UAOCombatAttributeSet::UAOCombatAttributeSet()
{
	
}

void UAOCombatAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UAOCombatAttributeSet , Attack , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOCombatAttributeSet , MaxSpeed , COND_None , REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAOCombatAttributeSet , SprintSpeedBonus , COND_None , REPNOTIFY_Always);
}

bool UAOCombatAttributeSet::PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data)
{
	return Super::PreGameplayEffectExecute(Data);
}

void UAOCombatAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	FEffectProperties Properties;
	SetEffectContext(Data, Properties);  //保存当前技能相关的上下文
	
	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();  //效果触发的始作俑者
	AActor* Causer = EffectContext.GetEffectCauser();  //获取触发效果的中间作用者
}

void UAOCombatAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
}

void UAOCombatAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	

}

void UAOCombatAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	
	if (Attribute == GetMaxSpeedAttribute() || Attribute == GetSprintSpeedBonusAttribute())
	{

		
		if (AAOCharacter* AOCharacter = Cast<AAOCharacter>( GetOwningAbilitySystemComponent()->GetAvatarActor()))
		{

			UCharacterMovementComponent* CharacterMovementComponent = AOCharacter->GetCharacterMovement();
			if (CharacterMovementComponent)
			{
				float TotalSpeed = GetMaxSpeed();
				CharacterMovementComponent->MaxWalkSpeed = TotalSpeed;
				float NewMaxWalkSpeed = CharacterMovementComponent->MaxWalkSpeed;
				UE_LOG(LogAegisOdysseyAttributeSet, Log, TEXT("UAOCombatAttributeSet::PostAttributeChange: MaxWalkSpeed  %.2f "), 
					NewMaxWalkSpeed);
			}
		}
	}
}

void UAOCombatAttributeSet::OnRep_Attack()
{
	
}

void UAOCombatAttributeSet::OnRep_MaxSpeed()
{
	UE_LOG(LogAegisOdysseyAbilitySystem, Log, TEXT("UAOCombatAttributeSet::OnRep_MaxSpeed: MaxSpeed replicated to %.2f"), GetMaxSpeed());
	
}

void UAOCombatAttributeSet::OnRep_SprintSpeedBonus()
{

}
