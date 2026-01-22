// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOPlayerController.h"
#include "ModularPlayerState.h"
#include "AegisOdyssey/GameModes/AOExperienceDefinition.h"
#include "AOPlayerState.generated.h"

/**
 * 
 */

class UAOPawnData;

UCLASS()
class AEGISODYSSEY_API AAOPlayerState : public AModularPlayerState
{
	GENERATED_BODY()
public:
	
	static const FName NAME_AOAbilityReady;
	
	UFUNCTION(BlueprintCallable, Category="AOPlayerStateConfig")
	AAOPlayerController* GetAOPlayerController() const;

	virtual void PostInitializeComponents() override;
	virtual void ClientInitialize(class AController* C) override;

	template<class T>
	const T* GetPawnData() const {return Cast<T>(PawnData);}

	void SetPawnData(const UAOPawnData* InPawnData);  //PawnDataSetter

private:
	void OnExperienceLoaded(const UAOExperienceDefinition* ExperienceDefinition);

protected:
	UFUNCTION()
	void OnRep_PawnData();
protected:
	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<const UAOPawnData> PawnData;
};


