// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/Abilities/AOGameplayAbility.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "GA_LightAttack.generated.h"

class UAbilityTask_PlayMontageAndWait;

/**
 * 轻攻击参数对象
 * 通过GameplayEvent的OptionalObject传递给GA_LightAttack
 */
UCLASS(BlueprintType)
class AEGISODYSSEY_API ULightAttackParams : public UObject
{
	GENERATED_BODY()

public:
	ULightAttackParams()
		: InputTag(FGameplayTag::EmptyTag)
		, InputType(EInputType::None)
		, Montage(nullptr)
		, PlayRate(1.0f)
		, StartSection(NAME_None)
		, StartTime(0.0f)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightAttack")
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightAttack")
	TEnumAsByte<EInputType> InputType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightAttack")
	class UAnimMontage* Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightAttack")
	float PlayRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightAttack")
	FName StartSection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LightAttack")
	float StartTime;
};

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UGA_LightAttack : public UAOGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_LightAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UFUNCTION()
	void PlayMontageAnimation();
	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageBlendedOut();
	UFUNCTION()
	void OnMontageInterrupted();
	UFUNCTION()
	void OnMontageCancelled();

private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	ULightAttackParams* Params;
};
