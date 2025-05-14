// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOExtPawnComponent.h"
#include "AbilitySystemInterface.h"
#include "ModularCharacter.h"
#include "AOCharacter.generated.h"

class USpringArmComponent;
class UAOCameraComponent;
/**
 * 
 */
UCLASS()
class AEGISODYSSEY_API AAOCharacter : public AModularCharacter,public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AAOCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UAOAbilitySystem* GetSourceASC() const {return AOSourceASC ? AOSourceASC : nullptr;}
private:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="AOCharacterConfig",meta=(AllowPrivateAccess=true))
	TObjectPtr<UAOExtPawnComponent> AOExtPawnComp;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="AOCharacterConfig",meta=(AllowPrivateAccess=true))
	TObjectPtr<UAOAbilitySystem> AOSourceASC;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="AOCharacterConfig",meta=(AllowPrivateAccess=true))
	TObjectPtr<UAOCameraComponent> AOCameraComponent;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="AOCharacterConfig",meta=(AllowPrivateAccess=true))
	TObjectPtr<USpringArmComponent> SpringArmComponent;
protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};
