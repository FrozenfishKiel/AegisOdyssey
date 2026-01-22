// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/Abilities/AOGameplayAbility.h"
#include "Abilities/GameplayAbility.h"
#include "AOGameplayAbility_Interact.generated.h"

struct FInteractionOption;
/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOGameplayAbility_Interact : public UAOGameplayAbility
{
	GENERATED_BODY()
public:
	UAOGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void UpdateInteractOptions(const TArray<FInteractionOption>& InteractionOptions);

	UFUNCTION(BlueprintCallable)
	void TriggerInteraction();

protected:

	UPROPERTY(BlueprintReadWrite)
	TArray<FInteractionOption> CurrentOptions;


protected:

	UPROPERTY(EditDefaultsOnly)
	float InteractionScanRate = 0.1f;

	UPROPERTY(EditDefaultsOnly)
	float InteractionScanRange = 500;

	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UUserWidget> DefaultInteractionWidgetClass;
};
