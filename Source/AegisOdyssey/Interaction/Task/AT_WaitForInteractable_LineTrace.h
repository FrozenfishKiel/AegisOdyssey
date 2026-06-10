// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AT_WaitForInteractableTargets.h"
#include "AT_WaitForInteractable_LineTrace.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAT_WaitForInteractable_LineTrace : public UAT_WaitForInteractableTargets
{
	GENERATED_BODY()

	virtual void Activate() override;
public:
	UFUNCTION(BlueprintCallable , Category = "Ability|Task" , meta = (HidePin = "OwingAbility" , DefaultToSelf = "OwingAbility" ,
		BlueprintInternalUseOnly = "TRUE"))
	static UAT_WaitForInteractable_LineTrace* WaitForInteractableTarget_SingleLineTrace(UGameplayAbility* OwingAbility ,
		FCollisionProfileName TraceProfile , FGameplayAbilityTargetingLocationInfo StartLocation ,
		float InteractionScanRange = 100.f , float InteractionScanRate = 0.100 , bool bShowDebug = false);
private:
	virtual void OnDestroy(bool bInOwnerFinished) override;

	void PerformTrace();

	UPROPERTY()
	FGameplayAbilityTargetingLocationInfo StartLocation;

	float InteractionScanRange = 100.f;
	float InteractionScanRate = 0.100f;
	bool bShowDebug = false;

	FTimerHandle TimerHandle;
};
