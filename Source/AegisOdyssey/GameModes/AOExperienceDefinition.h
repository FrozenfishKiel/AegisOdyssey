// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AOExperienceDefinition.generated.h"

class UAOPawnData;
/**
 * 
 */
class UGameFeatureAction;
UCLASS(BlueprintType, Const)
class AEGISODYSSEY_API UAOExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// List of actions to perform as this experience is loaded/activated/deactivated/unloaded
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Actions")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;  //加载所需的Action
	UPROPERTY(EditDefaultsOnly, Category="Actions")
	TArray<FString> GameFeatureNames;

	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TObjectPtr<const UAOPawnData> DefaultPawnData;
};
