// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AOExtPawnComponent.h"
#include "AbilitySystemInterface.h"
#include "ModularCharacter.h"
#include "AegisOdyssey/Inventory/AOBackPackComponent.h"
#include "AOCharacter.generated.h"

enum class ECharacterStates : uint8;
class UAOQuickBarComponent;
class UAOWeaponManagerComponent;
class UAOInventoryComponent;
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
	static const FName NAME_AOAbilityReady;
	AAOCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	UFUNCTION(BlueprintCallable)
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
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="AOCharacterConfig",meta=(AllowPrivateAccess=true))
	TObjectPtr<UAOWeaponManagerComponent> WeaponInventoryManager;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="AOCharacterConfig",meta=(AllowPrivateAccess=true))
	TObjectPtr<UAOBackPackComponent> CharacterBackPackComponent;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="AOCharacterConfig",meta=(AllowPrivateAccess=true))
	TObjectPtr<UAOQuickBarComponent> CharacterQuickBar;

protected:
	void DisableMovementAndCollision();
	void HandleStateTreeChange();
protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	virtual void Reset() override;
	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	//UFUNCTION(BlueprintCallable, Category="AOCharacter")
	//void TestFunc(UPARAM(ref) int32& InOutValue , bool Testbool);
private:
	UPROPERTY()
	TObjectPtr<const class UAOHealthAttributeSet> HealthAttributes;
};
