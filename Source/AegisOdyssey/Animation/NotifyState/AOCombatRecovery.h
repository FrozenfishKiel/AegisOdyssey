// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AOCombatRecovery.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOCombatRecovery : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UAOCombatRecovery();

	UFUNCTION(BlueprintCallable)
	FGameplayTag GetCombatWindowTag() const {return CombatRecoveryTag;}
protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
private:
	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly , meta = (AllowPrivateAccess = "true"))
	FGameplayTag CombatRecoveryTag;  //赋予玩家战斗后摇标签
};
