// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/Abilities/AOGameplayAbility.h"
#include "GA_ParriedReact.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UGameplayEffect;

UCLASS()
class AEGISODYSSEY_API UGA_ParriedReact : public UAOGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_ParriedReact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageBlendedOut();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnAllowMoveEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnFinishEvent(FGameplayEventData Payload);

private:
	void PlayMontageAnimation();
	void ApplyActivateEffectToSelf();
	void ReleaseMoveLock() const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParriedReact", meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UAnimMontage> ReactMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ParriedReact", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> ActivateEffectClass;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitAllowMoveTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitFinishTask;

	UPROPERTY()
	TObjectPtr<UAnimMontage> Montage = nullptr;
};
