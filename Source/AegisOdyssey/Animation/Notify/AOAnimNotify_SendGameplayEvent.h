// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AOAnimNotify_SendGameplayEvent.generated.h"

/**
 * Sends a single GameplayEvent to the mesh owner when this notify fires.
 */
UCLASS(DisplayName = "AO Send Gameplay Event")
class AEGISODYSSEY_API UAOAnimNotify_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAOAnimNotify_SendGameplayEvent();

	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Gameplay Event", meta = (Categories = "GameplayEvent", AllowPrivateAccess = "true"))
	FGameplayTag EventTag;
};
