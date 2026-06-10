// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Player/AAOAIPlayerBotController.h"
#include "Interface/AOEnemyPercepInterface.h"
#include "AOEnemyBotController.generated.h"

UCLASS()
class AEGISODYSSEY_API AAOEnemyBotController : public AAOAIPlayerBotController , public IAOEnemyPercepInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAOEnemyBotController();

	virtual AActor* GetSenseResultActor_Implementation() const override {return FocusActor.Get();}
	virtual void SetSenseResultActor_Implementation(AActor* SenseResultActor) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:
	UPROPERTY()
	TWeakObjectPtr<AActor> FocusActor;
};
