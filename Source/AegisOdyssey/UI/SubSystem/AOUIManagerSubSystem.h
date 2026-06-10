// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameUIManagerSubsystem.h"
#include "AOUIManagerSubSystem.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOUIManagerSubSystem : public UGameUIManagerSubsystem
{
	GENERATED_BODY()
public:
	UAOUIManagerSubSystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize();

private:
	bool Tick(float DeltaTime);
	void SyncRootLayoutVisibilityToShowHUD();

	FTSTicker::FDelegateHandle TickHandle;
};
