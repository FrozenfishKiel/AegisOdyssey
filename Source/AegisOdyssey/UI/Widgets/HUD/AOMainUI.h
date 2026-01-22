// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "AegisOdyssey/UI/ViewModel/MVVM_HUD.h"
#include "Blueprint/UserWidget.h"
#include "AOMainUI.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOMainUI : public UCommonUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	UFUNCTION(BlueprintPure , BlueprintCallable)
	UMVVM_HUD* GetMainHUDViewModel() const;
};
