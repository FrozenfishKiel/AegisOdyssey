// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/AOActivatableWidget.h"
#include "AOLayout_Escape.generated.h"

class UMVVM_Escape;
/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOLayout_Escape : public UAOActivatableWidget
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;
private:
	void HandleEscapeAction();
	UFUNCTION(BlueprintPure)
	UMVVM_Escape* GetEscapeViewModel() const;
};
