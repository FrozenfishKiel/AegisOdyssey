// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AOPublicFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOPublicFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static int32 GetPlayerLevel(UObject* ContextObject);
};
