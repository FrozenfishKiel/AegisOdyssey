// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InteractableTarget.h"
#include "InteractionStatics.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UInteractionStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UInteractionStatics();
public:
	UFUNCTION(BlueprintCallable)
	static AActor* GetActorFromInteractableTarget(TScriptInterface<IInteractableTarget> InteractableTarget);

	static void AppendInteractableTargetsFromHitResult(const FHitResult& HitResult, TArray<TScriptInterface<IInteractableTarget>>& OutInteractableTargets);
};
