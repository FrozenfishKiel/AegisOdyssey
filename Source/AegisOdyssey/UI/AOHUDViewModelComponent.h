#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "ViewModel/MVVM_HUD.h"
#include "AOHUDViewModelComponent.generated.h"
class UMVVM_HUD;

UCLASS()
class UAOHUDViewModelComponent : public UActorComponent , public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()
	UAOHUDViewModelComponent();
public:
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	virtual void BeginPlay() override;
	void InitializeAllViewModels(); void ClearAllViewModels();

	void SetHUDViewModelParams(FPlayerMainHUDViewModelParams& PlayerMainHUDViewModelParams);
	UMVVM_HUD* GetHUDMVVM() const {return HUDViewModel;}
	virtual void CheckDefaultInitialization() override;

protected:
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnRegister() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	UPROPERTY()
	TObjectPtr<UMVVM_HUD> HUDViewModel;
};

