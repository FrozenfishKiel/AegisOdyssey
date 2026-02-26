// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AOAbilitySystem.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOAbilitySystem : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	void AbilityInputTagStarted(const FGameplayTag& InputTag);
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	void ClearAbilityInput();
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
protected:
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

	void TryActivateAbilitiesOnSpawn();

protected:
	TArray<FGameplayAbilitySpecHandle> InputStartedSpecHandles;
	// Handles to abilities that had their input pressed this frame.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// Handles to abilities that have their input held.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
};


