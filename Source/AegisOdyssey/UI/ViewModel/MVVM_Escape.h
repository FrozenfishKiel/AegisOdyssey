// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOMVVMViewModelBase.h"
#include "MVVMViewModelBase.h"
#include "MVVM_Escape.generated.h"

/**
 * 
 */
UCLASS(Blueprintable , DisplayName = "Escape ViewModel")
class AEGISODYSSEY_API UMVVM_Escape : public UAOMVVMViewModelBase
{
	
	GENERATED_BODY()
public:
	void SetWidgetName(const FText& InSlotName);
	UFUNCTION(BlueprintPure , FieldNotify)
	FText GetWidgetName() const {return WidgetName;}
private:
	
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess))
	FText WidgetName;
};
