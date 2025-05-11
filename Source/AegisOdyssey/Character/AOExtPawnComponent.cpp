// Fill out your copyright notice in the Description page of Project Settings.


#include "AOExtPawnComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOExtPawnComponent)

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
	//检查PawnData，Controller是否Possess，满足则进入DataAvailiable
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
		return Manager->HaveAllFeaturesReachedInitState(Pawn, AOGameplayTags::InitState_DataAvailable);
		//这个函数用于检查指定Actor的所有继承IGameFrameworkInitStateInterface的
		//组件是否都已达到所需的初始化状态(所有数据已完成复制可开始初始化)，并可排除特定功能 //目的是等待其他组件初始化进行，此组件的状态才会继续
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
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
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

void UAOExtPawnComponent::OnRep_PawnData()
{
	CheckDefaultInitialization();
}

void UAOExtPawnComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAOExtPawnComponent, PawnData);
}

void UAOExtPawnComponent::HandleControllerChange()
{
	if (AbilitySystemComponent && (AbilitySystemComponent->GetAvatarActor() == GetPawnChecked<APawn>()))
	{
		ensure(AbilitySystemComponent->AbilityActorInfo->OwnerActor == AbilitySystemComponent->GetOwnerActor());
		if (AbilitySystemComponent->GetOwnerActor() == nullptr)
		{
			//UninitializeAbilitySystem();  //如果ASC的owner(控制器)不一致且为空则卸载当前的ASC
		}
		else
		{
			AbilitySystemComponent->RefreshAbilityActorInfo();  //刷新ASC的Actor信息
		}
	}
	CheckDefaultInitialization();
}

void UAOExtPawnComponent::HandlePlayerStateReplicated()
{
	CheckDefaultInitialization();
}

void UAOExtPawnComponent::InitializeAbilitySystem(UAOAbilitySystem* InASC, AActor* InActor)
{
	check(InASC);
	check(InActor);

	if (AbilitySystemComponent == InASC)
	{
		return;
	}

	if (AbilitySystemComponent)
	{
		UninitializeAbilitySystem();
	}

	APawn* Pawn = GetPawn<APawn>();
	AActor* ExistingAvatar = InASC->GetAvatarActor();

	AbilitySystemComponent = InASC;

	AbilitySystemComponent->InitAbilityActorInfo(InActor, Pawn);
}

//卸载ASC
void UAOExtPawnComponent::UninitializeAbilitySystem()
{
	if (!AbilitySystemComponent) return;
	AbilitySystemComponent = nullptr;
}

