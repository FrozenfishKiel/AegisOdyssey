// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "AOGameMode.generated.h"


class UGameFeatureAction;
class UAOGameFeatureDefinition;
class UGameFeaturesSubsystem;
/**
 * 
 */
namespace UE::GameFeatures { struct FResult; }
UCLASS(Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class AEGISODYSSEY_API AAOGameMode : public AModularGameModeBase
{
	GENERATED_BODY()
public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	void HandleGameInitialize();
	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);
	void OnGameFeaturePluginLoaded();
	void OnGameFeatureLoadedReady();
public:
	UPROPERTY(EditDefaultsOnly, Category="Actions")
	TObjectPtr<UAOGameFeatureDefinition> GameFeatureDefinition;
private:
	int32 NumGameFeaturePluginsLoading = 0;
	TArray<FString> GameFeaturePluginURLs;
};
