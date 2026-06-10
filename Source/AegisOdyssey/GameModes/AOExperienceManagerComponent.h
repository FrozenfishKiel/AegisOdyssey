// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOExperienceDefinition.h"
#include "Components/GameStateComponent.h"
#include "AOExperienceManagerComponent.generated.h"

class UGameFeatureAction;
class UAOExperienceDefinition;
class UGameFeaturesSubsystem;
/**
 * 
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnExperienceLoaded,const UAOExperienceDefinition*);

enum class EAOExperienceLoadState
{
	Unloaded,
	Loading,
	LoadingGameFeatures,
	LoadingChaosTestingDelay,
	ExecutingActions,
	Loaded,
	Deactivating
};
namespace UE::GameFeatures { struct FResult; }
UCLASS()
class AEGISODYSSEY_API UAOExperienceManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()
public:
	UAOExperienceManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	// Tries to set the current experience, either a UI or gameplay one
	void SetCurrentExperience(FPrimaryAssetId ExperienceId);
	void SetCurrentExperience(const TSubclassOf<UAOExperienceDefinition> ExperienceRef);
	void OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result);
	void OnGameFeaturePluginLoaded();
	void OnGameFeatureLoadedReady();
	void StartExperienceLoad();

	bool IsExperienceLoaded() const;

	const UAOExperienceDefinition* GetCurrentExperienceCheck() const;  //Experience Getter
public:
	void CallRegister_OnExperienceLoaded(FOnExperienceLoaded::FDelegate&& OnExperienceLoadedDelegate);
private:
	UFUNCTION()
	void OnRep_CurrentExperience();
private:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentExperience)
	TObjectPtr<const UAOExperienceDefinition> CurrentExperience;
	EAOExperienceLoadState LoadState = EAOExperienceLoadState::Unloaded;

	FOnExperienceLoaded OnExperienceLoaded;
private:
	int32 NumGameFeaturePluginsLoading = 0;
	TArray<FString> GameFeaturePluginURLs;
};
