// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Base/AOAttributeSet.h"
#include "AOPrimaryAttributeSet.generated.h"

class UAOPawnData;

/**
 *   角色的主要属性点，也就是加点属性，通常这作为ARPG的重要人物属性
 */
UCLASS()
class AEGISODYSSEY_API UAOPrimaryAttributeSet : public UAOAttributeSet
{
	GENERATED_BODY()
public:
	UAOPrimaryAttributeSet();
	// 力量值，用于提升物理攻击力和负重能力
	ATTRIBUTE_ACCESSORS(UAOPrimaryAttributeSet , Strength);
	// 智慧，魔法攻击力，也会在一定程度增加阅读速度
	ATTRIBUTE_ACCESSORS(UAOPrimaryAttributeSet , Intelligence);
	// 敏捷，一定程度上提高闪避能力
	ATTRIBUTE_ACCESSORS(UAOPrimaryAttributeSet , Agility);
	// 体质，提升生命值和耐力
	ATTRIBUTE_ACCESSORS(UAOPrimaryAttributeSet , Constitution);
	// 灵巧，提升暴击率
	ATTRIBUTE_ACCESSORS(UAOPrimaryAttributeSet , Dexterity);
	// 角色基础受击阈值，可随等级成长
	ATTRIBUTE_ACCESSORS(UAOPrimaryAttributeSet , HitReactThreshold);

	bool RefreshPrimaryAttributesFromLevel(const UAOPawnData* PawnData, int32 Level) const;
	
	mutable FAOAttributeEvent OnStrengthChanged;
	mutable FAOAttributeEvent OnIntelligenceChanged;
	mutable FAOAttributeEvent OnAgilityChanged;
	mutable FAOAttributeEvent OnConstitutionChanged;
	mutable FAOAttributeEvent OnDexterityChanged;

	
protected:
	void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
private:
	
	UPROPERTY(BlueprintReadOnly , ReplicatedUsing = OnRep_Strength,  Category = "AO | Strength",meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Strength;
	UFUNCTION()
	void OnRep_Strength(FGameplayAttributeData const & OldValue);
	
	UPROPERTY(BlueprintReadOnly , ReplicatedUsing = OnRep_Intelligence,  Category = "AO | Intelligence",meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Intelligence;
	UFUNCTION()
	void OnRep_Intelligence(FGameplayAttributeData const & OldValue);
	
	UPROPERTY(BlueprintReadOnly , ReplicatedUsing = OnRep_Agility,  Category = "AO | Agility",meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Agility;
	UFUNCTION()
	void OnRep_Agility(FGameplayAttributeData const & OldValue);
	
	UPROPERTY(BlueprintReadOnly , ReplicatedUsing = OnRep_Constitution,Category = "AO | Constitution",meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Constitution;
	UFUNCTION()
	void OnRep_Constitution(FGameplayAttributeData const & OldValue);
	
	UPROPERTY(BlueprintReadOnly , ReplicatedUsing = OnRep_Dexterity , Category = "AO | Dexterity",meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Dexterity;
	UFUNCTION()
	void OnRep_Dexterity(FGameplayAttributeData const & OldValue);

	UPROPERTY(BlueprintReadOnly , ReplicatedUsing = OnRep_HitReactThreshold , Category = "AO | HitReact",meta = (AllowPrivateAccess = true, ClampMin = "0.0"))
	FGameplayAttributeData HitReactThreshold;
	UFUNCTION()
	void OnRep_HitReactThreshold(FGameplayAttributeData const & OldValue);
};
