// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMSubsystem.h"
#include "AOMVVMSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOMVVMSubsystem : public UMVVMSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

};
