// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularCharacter.h"
#include "Components/GameFrameworkComponentManager.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularCharacter)
//快速编译宏

// Sets default values
AModularCharacter::AModularCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AModularCharacter::BeginPlay()
{
	Super::BeginPlay();
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this,UGameFrameworkComponentManager::NAME_GameActorReady);
	//游戏开始时，告诉动态组件添加管理器该Actor已就绪
}

void AModularCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this); //组件初始化之前，添加该actor为注册动态组件事件
}

void AModularCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
	//游戏结束时，移除该Actor的注册
	Super::EndPlay(EndPlayReason);
}


