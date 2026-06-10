// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "AOAbilitySystem.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_AbilityInputBlocked);

class UAttributeSet;
DECLARE_MULTICAST_DELEGATE(FOnAOAbilitySystemDataChanged);

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
	UAttributeSet* EnsureSpawnedAttributeSet(TSubclassOf<UAttributeSet> AttributeSetClass);
	void TryActivateAbilitiesOnSpawn();
	FOnAOAbilitySystemDataChanged& OnAbilitySystemDataChanged() { return AbilitySystemDataChangedDelegate; }

protected:
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;
	virtual void OnRep_ActivateAbilities() override;
	void GatherAbilityHandlesForInputTag(const FGameplayTag& InputTag, TArray<FGameplayAbilitySpecHandle>& OutAbilityHandles) const;

protected:
	TArray<FGameplayAbilitySpecHandle> InputStartedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
	FOnAOAbilitySystemDataChanged AbilitySystemDataChangedDelegate;
};
