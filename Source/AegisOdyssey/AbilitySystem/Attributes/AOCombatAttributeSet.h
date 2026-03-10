// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "AOAttributeSet.h"
#include "AOCombatAttributeSet.generated.h"

/**
 * 
 */

UCLASS()
class AEGISODYSSEY_API UAOCombatAttributeSet : public UAOAttributeSet
{
	GENERATED_BODY()

public:
	UAOCombatAttributeSet();

	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet , Attack);
	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet , MaxSpeed);
	ATTRIBUTE_ACCESSORS(UAOCombatAttributeSet , SprintSpeedBonus);

protected:
	void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

private:

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Attack, Category = "AO|Attack", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Attack;
	UFUNCTION()
	void OnRep_Attack();
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxSpeed, Category = "AO|Movement", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxSpeed;
	UFUNCTION()
	void OnRep_MaxSpeed();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SprintSpeedBonus, Category = "AO|Movement", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData SprintSpeedBonus;
	UFUNCTION()
	void OnRep_SprintSpeedBonus();
};
