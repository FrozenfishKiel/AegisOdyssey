// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AOGiveOwnerTagWindow.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOGiveOwnerTagWindow : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UAOGiveOwnerTagWindow();
	UFUNCTION(BlueprintCallable)
	virtual FGameplayTag GetWindowTag() const {return GiveTag;}
protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
private:
	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly , meta = (AllowPrivateAccess = "true"))
	FGameplayTag GiveTag;
};
