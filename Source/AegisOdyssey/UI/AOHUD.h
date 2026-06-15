// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AOHUD.generated.h"

class UAOHUDViewModelComponent;
class SWidget;
/**
 * 
 */
UCLASS(Config = Game)
class AEGISODYSSEY_API AAOHUD : public AHUD
{
	GENERATED_BODY()
public:
	AAOHUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	template<typename TargetClass>
	static TargetClass* FindHUDOwnedComponent(const APlayerController* LocalPlayerController) {return LocalPlayerController->IsLocalController() ? LocalPlayerController->GetHUD()->FindComponentByClass<TargetClass>() : nullptr;}

	static void SetAIDebugPanelEnabledForWorld(UWorld* World, bool bEnabled);
	void SetAIDebugPanelEnabled(bool bEnabled);
	bool IsAIDebugPanelEnabled() const { return bAIDebugPanelEnabled; }
protected:
	//~UObject interface
	virtual void PreInitializeComponents() override;
	virtual void PostLoad() override;
	//~End of UObject interface

	//~AActor interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface
protected:
	UPROPERTY()
	TObjectPtr<UAOHUDViewModelComponent> HUDViewModelComponent;

private:
	void ShowAIDebugPanel();
	void HideAIDebugPanel();

private:
	bool bAIDebugPanelEnabled = false;
	TSharedPtr<SWidget> AIDebugPanelWidget;
};
