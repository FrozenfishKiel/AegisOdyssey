// Fill out your copyright notice in the Description page of Project Settings.


#include "AOHUD.h"

#include "AOHUDViewModelComponent.h"
#include "Components/GameFrameworkComponentManager.h"
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
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
	
	Super::EndPlay(EndPlayReason);
}
