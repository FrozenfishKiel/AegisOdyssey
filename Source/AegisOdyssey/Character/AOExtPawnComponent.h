// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOPawnData.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "AOExtPawnComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, Meta=(BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOExtPawnComponent : public UPawnComponent , public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()
public:
	static const FName NAME_ActorFeatureName;
	UAOExtPawnComponent(const FObjectInitializer& ObjectInitializer);

	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	static UAOExtPawnComponent* FindAOExtPawnComponent(const AActor* Actor){return Actor ? Actor->FindComponentByClass<UAOExtPawnComponent>() : nullptr;}

	/** Should be called by the owning pawn when the input component is setup. */
	void SetupPlayerInputComponent();
	void HandleControllerChange();
	void HandlePlayerStateReplicated();
	void InitializeAbilitySystem(UAOAbilitySystem* InASC, AActor* InActor);
	void UninitializeAbilitySystem(); //卸载 ASC

	//返回创建的对应类的PawnData
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }
protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_PawnData, Category = "AO|Pawn")
	TObjectPtr<const UAOPawnData> PawnData;
	UFUNCTION()
	void OnRep_PawnData();
private:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
