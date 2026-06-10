// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AOEnemy.generated.h"

UCLASS()
class AEGISODYSSEY_API AAOEnemy : public AAOCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAOEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
