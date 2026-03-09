// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AOCombatPreparation.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOCombatPreparation : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UAOCombatPreparation();
	UFUNCTION(BlueprintCallable)
	FGameplayTag GetCombatWindowTag() const {return CombatPreparationTag;}
protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
private:
	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly , meta = (AllowPrivateAccess = "true"))
	FGameplayTag CombatPreparationTag;
};
