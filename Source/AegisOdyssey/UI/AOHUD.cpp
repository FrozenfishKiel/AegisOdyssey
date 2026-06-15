// Fill out your copyright notice in the Description page of Project Settings.


#include "AOHUD.h"

#include "AOHUDViewModelComponent.h"
#include "AegisOdyssey/UI/AIDebug/SAOAIDecisionDebugPanel.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/SWeakWidget.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHUD)

AAOHUD::AAOHUD(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	HUDViewModelComponent = CreateDefaultSubobject<UAOHUDViewModelComponent>(TEXT("HUDViewModelComponent"));
}

void AAOHUD::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void AAOHUD::PostLoad()
{
	Super::PostLoad();
}

void AAOHUD::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this,UGameFrameworkComponentManager::NAME_GameActorReady);
	
	Super::BeginPlay();
}

void AAOHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideAIDebugPanel();
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
	
	Super::EndPlay(EndPlayReason);
}

void AAOHUD::SetAIDebugPanelEnabledForWorld(UWorld* World, bool bEnabled)
{
	if (World == nullptr)
	{
		return;
	}

	for (TActorIterator<AAOHUD> It(World); It; ++It)
	{
		if (AAOHUD* HUD = *It)
		{
			HUD->SetAIDebugPanelEnabled(bEnabled);
		}
	}
}

void AAOHUD::SetAIDebugPanelEnabled(bool bEnabled)
{
	if (bAIDebugPanelEnabled == bEnabled)
	{
		return;
	}

	bAIDebugPanelEnabled = bEnabled;

	if (HUDViewModelComponent != nullptr)
	{
		HUDViewModelComponent->SetAIDebugObservationEnabled(bAIDebugPanelEnabled);
	}

	if (bAIDebugPanelEnabled)
	{
		ShowAIDebugPanel();
	}
	else
	{
		HideAIDebugPanel();
	}
}

void AAOHUD::ShowAIDebugPanel()
{
	if (AIDebugPanelWidget.IsValid())
	{
		return;
	}

	if (GetOwningPlayerController() == nullptr || !GetOwningPlayerController()->IsLocalController())
	{
		return;
	}

	if (HUDViewModelComponent == nullptr || HUDViewModelComponent->GetAIDecisionDebugViewModel() == nullptr)
	{
		return;
	}

	UGameViewportClient* GameViewport = GetWorld() != nullptr ? GetWorld()->GetGameViewport() : nullptr;
	if (GameViewport == nullptr)
	{
		return;
	}

	AIDebugPanelWidget =
		SNew(SAOAIDecisionDebugPanel)
		.DebugViewModel(HUDViewModelComponent->GetAIDecisionDebugViewModel());

	GameViewport->AddViewportWidgetContent(
		SNew(SWeakWidget)
		.PossiblyNullContent(AIDebugPanelWidget.ToSharedRef()),
		1000);
}

void AAOHUD::HideAIDebugPanel()
{
	if (!AIDebugPanelWidget.IsValid())
	{
		return;
	}

	if (UGameViewportClient* GameViewport = GetWorld() != nullptr ? GetWorld()->GetGameViewport() : nullptr)
	{
		GameViewport->RemoveViewportWidgetContent(AIDebugPanelWidget.ToSharedRef());
	}

	AIDebugPanelWidget.Reset();
}
