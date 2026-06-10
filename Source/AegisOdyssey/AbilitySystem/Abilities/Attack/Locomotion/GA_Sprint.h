// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/AbilitySystem/Abilities/AOGameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GA_Sprint.generated.h"

class UAT_WaitVigorExhaust;
class UGameplayEffect;
UCLASS()
class AEGISODYSSEY_API UGA_Sprint : public UAOGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Sprint(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float SprintSpeedBonusAmount = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float SprintCost = 0.f;
	bool bVigorExhaustedBroadcasted = false;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> SprintCostVigorClass;

protected:
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
private:
	void InitializeVigorCost();
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
	void OnVigorWasExhausted();
	void OnCharacterMoved();
	void OnCharacterStopped();
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitInputRelease> WaitInputReleaseTask;
	UPROPERTY()
	TObjectPtr<UAT_WaitVigorExhaust> WaitVigorExhaustTask;

	FActiveGameplayEffectHandle SprintSpeedEffectHandle;
	
	FGameplayEffectSpec CostSpec;
	FActiveGameplayEffectHandle ActiveSprintVigorCostHandle;

};


DECLARE_MULTICAST_DELEGATE(FWaitVigorExhaust)
DECLARE_MULTICAST_DELEGATE(FOnCharacterMove)
DECLARE_MULTICAST_DELEGATE(FOnCharacterStop)

//等待体力消耗
UCLASS()
class AEGISODYSSEY_API UAT_WaitVigorExhaust : public UAbilityTask
{
	GENERATED_BODY()
public:
	UAT_WaitVigorExhaust(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION(BlueprintCallable,Category = "Ability|Tasks",meta = (HidePin = "OwningAbility",DefaultToSelf = "OwningAbility" , BlueprintInternalUseOnly = "TRUE"))
	static UAT_WaitVigorExhaust* CreateWaitVigorExhaust(UAOGameplayAbility* OwningAbility);
public:
	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
	FWaitVigorExhaust OnWaitVigorExhaust;
	FOnCharacterMove OnCharacterMove;
	FOnCharacterStop OnCharacterStop;

};