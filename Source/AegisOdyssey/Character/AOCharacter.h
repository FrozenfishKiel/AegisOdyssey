// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOExtPawnComponent.h"
#include "AbilitySystemInterface.h"
#include "ModularCharacter.h"
#include "AOCharacter.generated.h"

/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API AAOCharacter : public AModularCharacter
{
	GENERATED_BODY()
public:
	AAOCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
private:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="AOCharacterConfig",meta=(AllowPrivateAccess=true))
	TObjectPtr<UAOExtPawnComponent> AOExtPawnComp;
};
