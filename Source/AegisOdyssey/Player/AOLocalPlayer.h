// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonLocalPlayer.h"
#include "AOLocalPlayer.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOLocalPlayer : public UCommonLocalPlayer
{
	GENERATED_BODY()
public:
	virtual void SwitchController(class APlayerController* PC) override;
	virtual bool SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld) override;

protected:
	void OnControllerChanged(APlayerController* NewController);
};
