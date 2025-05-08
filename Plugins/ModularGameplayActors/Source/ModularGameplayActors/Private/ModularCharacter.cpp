// Copyright Epic Games, Inc. All Rights Reserved.

#include "ModularCharacter.h"
#include "Components/GameFrameworkComponentManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularCharacter)

void AModularCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
	//在组件初始化之前，订阅/注册游戏框架组件管理器，使其能够接受到组件生命周期相关事件通知，例如组件添加，初始化，销毁的时候
}

void AModularCharacter::BeginPlay()
{
	//游戏开始的时候，发送事件，发送Actor组件已经准备就绪的事件
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();
}

void AModularCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

