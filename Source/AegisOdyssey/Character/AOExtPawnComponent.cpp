// Fill out your copyright notice in the Description page of Project Settings.


#include "AOExtPawnComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "AegisOdyssey/AOGameplayTags.h"

const FName UAOExtPawnComponent::NAME_ActorFeatureName("PawnExtension");

void UAOExtPawnComponent::CheckDefaultInitialization()
{
	//该函数会尝试推进注册的Actor下其他Component的初始化阶段，之后会继续推进自己的初始化阶段
	CheckDefaultInitializationForImplementers();

	static const TArray<FGameplayTag> StateChain = {AOGameplayTags::InitState_Spawned,AOGameplayTags::InitState_DataAvailable,AOGameplayTags::InitState_DataInitialized,
	AOGameplayTags::InitState_GameplayReady};

	//导入状态改变链，按顺序依次变更状态，每次变更都会调用HandleChangeInitState，在此之前要经过CanChangeInitState的检查
	ContinueInitStateChain(StateChain);
}

//判断是否可以改变初始化的状态
bool UAOExtPawnComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState) const
{
	check(Manager);
	APawn* Pawn = GetPawn<APawn>();  //获取拥有该组件的Pawn
	//若当前状态为空且目标状态为Actor已生成但未初始化
	if (!CurrentState.IsValid() && DesiredState == AOGameplayTags::InitState_Spawned)
	{
		if (Pawn)
		{
			return true;
		}
	}
	//若当前状态等于Actor/组件已生成但未初始化，且目标状态为所有数据已加载或复制完成，可开始初始化
	if (CurrentState == AOGameplayTags::InitState_Spawned && DesiredState == AOGameplayTags::InitState_DataAvailable)
	{
		const bool bHasAuthority = Pawn->HasAuthority();
		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();

		if (bHasAuthority && bIsLocallyControlled)
		{
			if (!GetController<AController>())
			{
				return false;
			}
		}
		return true;
	}
	//如果当前状态为所需数据已加载或复制完成，可开始初始化且目标状态为数据已初始化但尚未准备好参与核心游戏玩法时
	else if (CurrentState == AOGameplayTags::InitState_DataAvailable && DesiredState == AOGameplayTags::InitState_DataInitialized)
	{
		// Transition to initialize if all features have their data available
		return Manager->HaveAllFeaturesReachedInitState(Pawn, AOGameplayTags::InitState_DataAvailable);
		//这个函数用于检查指定Actor的所有功能组件是否都已达到所需的初始化状态(所有数据已完成复制可开始初始化)，并可排除特定功能
	}
	else if (CurrentState == AOGameplayTags::InitState_DataInitialized && DesiredState == AOGameplayTags::InitState_GameplayReady)
	{
		return true;
	}
	
	return false;
}

void UAOExtPawnComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	IGameFrameworkInitStateInterface::HandleChangeInitState(Manager, CurrentState, DesiredState);
}
//用BindOnActorInitStateChanged()绑定的回调
void UAOExtPawnComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == NAME_ActorFeatureName)
	{
		//如果FeatureState仍处于可开始初始化状态，则继续推进状态
		if (Params.FeatureState == AOGameplayTags::InitState_DataAvailable)
		{
			CheckDefaultInitialization();
		}
	}
}

//角色被控制器控制且输入组件准备就绪时调用，推进状态
void UAOExtPawnComponent::SetupPlayerInputComponent()
{
	CheckDefaultInitialization();
}

void UAOExtPawnComponent::OnRegister()
{
	Super::OnRegister();
	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("LyraPawnExtensionComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));
	//检查玩家拥有的组件是否超过1个
	TArray<UActorComponent*> PawnExtensionComponents;
	Pawn->GetComponents(UAOExtPawnComponent::StaticClass(), PawnExtensionComponents);
	ensureAlwaysMsgf((PawnExtensionComponents.Num() == 1), TEXT("Only one LyraPawnExtensionComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	RegisterInitStateFeature();  //注册初始化状态链
}

void UAOExtPawnComponent::BeginPlay()
{
	Super::BeginPlay();
	// 监听Actor初始化状态的更改
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);
	
	// Notifies state manager that we have spawned, then try rest of default initialization
	ensure(TryToChangeInitState(AOGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UAOExtPawnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	UnregisterInitStateFeature();
}

void UAOExtPawnComponent::HandleControllerChange()
{
	CheckDefaultInitialization();
}

