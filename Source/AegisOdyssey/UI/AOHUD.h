// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AOHUD.generated.h"

/**
 * 
 */
UCLASS(Config = Game)
class AEGISODYSSEY_API AAOHUD : public AHUD
{
	GENERATED_BODY()
public:
	AAOHUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~UObject interface
	virtual void PreInitializeComponents() override;
	virtual void PostLoad() override;
	//~End of UObject interface

	//~AActor interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface
	
};
