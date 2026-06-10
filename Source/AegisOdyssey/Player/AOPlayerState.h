// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularPlayerState.h"
#include "AegisOdyssey/GameModes/AOExperienceDefinition.h"
#include "AOPlayerState.generated.h"

class AAOCharacter;
class AAOPlayerController;
class UAOPawnData;

UCLASS()
class AEGISODYSSEY_API AAOPlayerState : public AModularPlayerState
{
	GENERATED_BODY()

public:
	static const FName NAME_AOAbilityReady;

	/** 返回当前 PlayerState 对应的 AO 玩家控制器。 */
	UFUNCTION(BlueprintCallable, Category = "AOPlayerStateConfig")
	AAOPlayerController* GetAOPlayerController() const;

	virtual void PostInitializeComponents() override;
	virtual void ClientInitialize(class AController* C) override;

	template<class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	void SetPawnData(const UAOPawnData* InPawnData);

	UFUNCTION(BlueprintCallable, Category = "AO|Level")
	int32 GetCharacterLevel() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Level")
	int32 GetCharacterXP() const;

	UFUNCTION(BlueprintCallable, Category = "AO|Level")
	int32 GetAvailableAttributePoints() const;

	void SetCharacterLevel(int32 NewLevel);
	void AddToCharacterLevel(int32 DeltaLevel);

	void SetCharacterXP(int32 NewXP);
	void AddToCharacterXP(int32 DeltaXP);

	void SetAvailableAttributePoints(int32 NewPoints);
	void AddToAvailableAttributePoints(int32 DeltaPoints);

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLevelChanged, int32, int32);
	FOnLevelChanged OnLevelChangedDelegate;

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnXPChanged, int32, int32);
	FOnXPChanged OnXPChangedDelegate;

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAttributePointsChanged, int32, int32);
	FOnAttributePointsChanged OnAttributePointsChangedDelegate;

private:
	void OnExperienceLoaded(const UAOExperienceDefinition* ExperienceDefinition);
	AAOCharacter* GetControlledCharacter() const;

	UFUNCTION()
	void OnPawnChanged(APawn* OldPawn, APawn* NewPawn);

	void BindCharacterDelegates(AAOCharacter* Character);
	void UnbindCharacterDelegates(AAOCharacter* Character);

	UPROPERTY()
	TObjectPtr<AAOCharacter> CurrentControlledCharacter;

protected:
	UFUNCTION()
	void OnRep_PawnData();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_PawnData)
	TObjectPtr<const UAOPawnData> PawnData;
};
