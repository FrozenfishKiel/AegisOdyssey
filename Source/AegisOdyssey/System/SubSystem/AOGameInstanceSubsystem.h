// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/UI/ViewModel/AOMVVMViewModelBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AOGameInstanceSubsystem.generated.h"

class UMVVM_Escape;
/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
public:
	UFUNCTION(BlueprintPure , BlueprintCallable)
	UMVVM_Escape* GetEscapeViewModel() const {return EscapeViewModel;} 
private:
	void InitializeViewModel();
private:
	UPROPERTY()
	TObjectPtr<UMVVM_Escape> EscapeViewModel;
};
