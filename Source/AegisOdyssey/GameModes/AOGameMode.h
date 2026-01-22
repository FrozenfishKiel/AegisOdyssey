// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "AOGameMode.generated.h"


class UAOPawnData;
class UGameFeatureAction;
class UAOExperienceDefinition;
class UGameFeaturesSubsystem;
/**
 * 
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAOGameModePlayerInitialized, AGameModeBase* /*GameMode*/, AController* /*NewPlayer*/);

namespace UE::GameFeatures { struct FResult; }
UCLASS(Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class AEGISODYSSEY_API AAOGameMode : public AModularGameModeBase
{
	GENERATED_BODY()
public:
	AAOGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	const UAOPawnData* GetPawnDataForController(const AController* InController) const;
	
	virtual void InitGameState() override;
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void FailedToRestartPlayer(AController* NewPlayer) override;
	virtual void GenericPlayerInitialization(AController* C) override;
	virtual void InitializeHUDForPlayer_Implementation(APlayerController* NewPlayer) override;
	void HandleGameInitialize();

	virtual bool ControllerCanRestart(AController* Controller);

	// Restart (respawn) the specified player or bot next frame
	// - If bForceReset is true, the controller will be reset this frame (abandoning the currently possessed pawn, if any)
	UFUNCTION(BlueprintCallable)
	void RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset = false);

	FOnAOGameModePlayerInitialized OnAOGameModePlayerInitialized;

protected:
	void OnExperienceLoaded(const UAOExperienceDefinition* CurrentExperience);
	bool IsExperienceLoaded() const;
protected:
	void HandleMatchAssignmentIfNotExpectingOne();
};
