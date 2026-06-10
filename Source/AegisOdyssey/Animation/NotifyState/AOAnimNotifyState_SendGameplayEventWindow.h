// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "AOAnimNotifyState_SendGameplayEventWindow.generated.h"

/**
 * Sends optional begin/end GameplayEvents to the mesh owner across a notify window.
 */
UCLASS(DisplayName = "AO Gameplay Event Window")
class AEGISODYSSEY_API UAOAnimNotifyState_SendGameplayEventWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAOAnimNotifyState_SendGameplayEventWindow();

	virtual FString GetNotifyName_Implementation() const override;

protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Gameplay Event", meta = (Categories = "GameplayEvent", AllowPrivateAccess = "true"))
	FGameplayTag BeginEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AO|Gameplay Event", meta = (Categories = "GameplayEvent", AllowPrivateAccess = "true"))
	FGameplayTag EndEventTag;
};
