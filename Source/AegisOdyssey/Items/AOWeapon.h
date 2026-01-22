// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Items/AOItem.h"
#include "AOWeapon.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API AAOWeapon : public AAOItem
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual void InitializeActorSpawnConfig() override;

};
