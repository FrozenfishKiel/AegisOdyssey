// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AOInputBufferWindow.generated.h"

/**
 * 
 */
UCLASS(DisplayName = "InputBufferWindow")
class AEGISODYSSEY_API UAOInputBufferWindow : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UAOInputBufferWindow();
protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
