// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOGameplayAbility.h"
#include "GA_PlayAnimationMontage.generated.h"

class UAbilityTask_PlayAnimAndWait;
class UAbilityTask_PlayMontageAndWait;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FPlayAnimationMontageTargetData : public FGameplayAbilityTargetData
{
	GENERATED_BODY()
	FPlayAnimationMontageTargetData():DataMontage(nullptr) {}

	UPROPERTY(BlueprintReadWrite, Category = "PlayAnimMontage")
	TSoftObjectPtr<UAnimMontage> DataMontage;

	
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FPlayAnimationMontageTargetData::StaticStruct();
	}
	
	
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << DataMontage;
		bOutSuccess = true;
		return true;
	}
};


template<>
struct TStructOpsTypeTraits<FPlayAnimationMontageTargetData> : public TStructOpsTypeTraitsBase2<FPlayAnimationMontageTargetData>
{
	enum
	{
		WithNetSerializer = true,
	};
};
UCLASS()
class AEGISODYSSEY_API UGA_PlayAnimationMontage : public UAOGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_PlayAnimationMontage(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
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
	UFUNCTION()
	void OnMovementInputDetected();
private:
	UPROPERTY()
	UAnimMontage* Montage;
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> PlayMontageAndWaitTask;
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayAnimAndWait> PlayAnimAndWait;
};
