// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOAttributeSet.h"
#include "NativeGameplayTags.h"
#include "AOHealthAttributeSet.generated.h"


/**
 * 
 */

class UAOVMPawnComponent;
AEGISODYSSEY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_Damage);
AEGISODYSSEY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageImmunity);
AEGISODYSSEY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageSelfDestruct);
AEGISODYSSEY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_FellOutOfWorld);
AEGISODYSSEY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AO_Damage_Message);
struct FGameplayEffectModCallbackData;

UCLASS()
class AEGISODYSSEY_API UAOHealthAttributeSet : public UAOAttributeSet
{
	GENERATED_BODY()
public:
	UAOHealthAttributeSet();

	ATTRIBUTE_ACCESSORS(UAOHealthAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UAOHealthAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UAOHealthAttributeSet, Healing);
	ATTRIBUTE_ACCESSORS(UAOHealthAttributeSet, Damage);

	mutable FAOAttributeEvent OnHealthChange;

	mutable FAOAttributeEvent OnMaxHealthChange;

	mutable FAOAttributeEvent OnOutOfHealth;
protected:
	void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	inline UAOVMPawnComponent* GetAOVMPawnComponent();

private:
	UPROPERTY(BlueprintReadOnly , ReplicatedUsing = OnRep_Health,Category = "AO|Health" , Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Health;  //生命值
 
	UPROPERTY(BlueprintReadOnly , ReplicatedUsing = OnRep_MaxHealth,Category = "AO|MaxHealth" , Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHealth; //最大生命值
	
	bool bOutOfHealth = false;

	float MaxHealthBeforeAttributeChange;  //记录修改前的Current属性
	float HealthBeforeAttributeChange;

	/*Meta Attribute*/

	UPROPERTY(BlueprintReadOnly , Category = "AO|Healing" , Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Healing; //恢复生命值

	UPROPERTY(BlueprintReadOnly , Category = "AO|MaxHealth" , Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Damage; //伤害
};

