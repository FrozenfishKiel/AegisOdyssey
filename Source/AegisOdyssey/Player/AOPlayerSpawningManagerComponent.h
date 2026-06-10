// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOPlayerStart.h"
#include "Components/GameStateComponent.h"
#include "AOPlayerSpawningManagerComponent.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API UAOPlayerSpawningManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()
public:
	UAOPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);

	virtual AActor* OnChoosePlayerStart(AController* Player, TArray<AAOPlayerStart*>& PlayerStarts) { return nullptr; }
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);
	void HandleOnActorSpawned(AActor* SpawnedActor);
	AActor* ChoosePlayerStart(AController* Player);
	bool ControllerCanRestart(AController* Player);
	void FinishRestartPlayer(AController* NewPlayer , const FRotator& StartRotation);
	friend class AAOGameMode;

#if WITH_EDITOR
	AAOPlayerStart* FindPlayerFromHereStart(AController* Player);
#endif

protected:
	AAOPlayerStart* GetFirstRandomUnoccupiedPlayerStart(AController* Controller, const TArray<AAOPlayerStart*>& FoundStartPoints) const;
private:
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AAOPlayerStart>> CachedPlayerStarts;
};
