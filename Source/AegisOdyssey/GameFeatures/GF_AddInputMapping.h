// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction_WordActionBase.h"
#include "UObject/SoftObjectPtr.h"
#include "GF_AddInputMapping.generated.h"

/**
 * 
 */
class UInputMappingContext;
USTRUCT()
struct FInputMappingContextAndPriority
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Input", meta=(AssetBundles="Client,Server"))
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	// Higher priority input mappings will be prioritized over mappings with a lower priority.
	UPROPERTY(EditAnywhere, Category="Input")
	int32 Priority = 0;
	
	/** If true, then this mapping context will be registered with the settings when this game feature action is registered. */
	UPROPERTY(EditAnywhere, Category="Input")
	bool bRegisterWithSettings = true;
};
UCLASS(MinimalAPI, meta = (DisplayName = "Add Input Mapping"))
class UGF_AddInputMapping final : public UGameFeatureAction_WordActionBase
{
	GENERATED_BODY()
public:
	//~UGameFeatureAction interface
	virtual void OnGameFeatureRegistering() override;
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	virtual void OnGameFeatureUnregistering() override;
	//~End of UGameFeatureAction interface

	UPROPERTY(EditAnywhere, Category="Input")
	TArray<FInputMappingContextAndPriority> InputMappings;
private:
	FDelegateHandle RegisterInputContextMappingsForGameInstanceHandle;  //用于注册IMC于GameInstance的委托句柄
	void RegisterInputMappingContexts();
	void RegisterInputContextMappingsForGameInstance(UGameInstance* GameInstance);
	void RegisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer);

	void UnregisterInputMappingContexts();

	void UnregisterInputContextMappingsForGameInstance(UGameInstance* GameInstance);

	void UnregisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer);
};


