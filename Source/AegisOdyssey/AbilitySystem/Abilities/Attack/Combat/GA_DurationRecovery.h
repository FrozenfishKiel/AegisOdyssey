// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/Abilities/AOGameplayAbility.h"
#include "GA_DurationRecovery.generated.h"

/**
 * 
 */

//持续时间恢复技能，这个技能是一个OnSpawn技能，意味着玩家开始就会拥有
UCLASS()
class AEGISODYSSEY_API UGA_DurationRecovery : public UAOGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_DurationRecovery(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
public:
	UPROPERTY(EditAnywhere , BlueprintReadOnly , Category = "Cofig")
	float RecoverValue = 0.f;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UGameplayEffect> RecoveryEffectToApplyClass;
private:
	void ApplyEffectToAvatarActor();
	FActiveGameplayEffectHandle RecoveryEffectHandle;
};
