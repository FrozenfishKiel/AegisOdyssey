// Fill out your copyright notice in the Description page of Project Settings.


#include "AOHeroComponent.h"

#include "AOCharacter.h"
#include "AOExtPawnComponent.h"
#include "AegisOdyssey/AOGameplayTags.h"
#include "AegisOdyssey/Player/AOPlayerController.h"
#include "AegisOdyssey/Player/AOPlayerState.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/PlayerState.h"
const FName UAOHeroComponent::NAME_ActorFeatureName("Hero");
#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHeroComponent)
UAOHeroComponent::UAOHeroComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
}

//每阶段的状态推断
bool UAOHeroComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
                                          FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();

	if (!CurrentState.IsValid() && DesiredState == AOGameplayTags::InitState_Spawned)
	{
		if (Pawn)
		{
			return true;
		}
	}
	else if (CurrentState == AOGameplayTags::InitState_Spawned && DesiredState == AOGameplayTags::InitState_DataAvailable)
	{
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			AController* Controller = GetController<AController>();

			const bool bHasControllerPairedWithPS = (Controller != nullptr) && \
				(Controller->PlayerState != nullptr) && \
				(Controller->PlayerState->GetOwner() == Controller);

			if (!bHasControllerPairedWithPS)
			{
				return false;
			}
		}

		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const bool bIsBot = Pawn->IsBotControlled();

		if (bIsLocallyControlled && !bIsBot)
		{
			AAOPlayerController* AOPC = GetController<AAOPlayerController>();

			if (!Pawn->InputComponent || !AOPC || !AOPC->GetLocalPlayer())
			{
				return false;
			}
		}
		return true;
	}
	else if (CurrentState == AOGameplayTags::InitState_DataAvailable && DesiredState == AOGameplayTags::InitState_DataInitialized)
	{
		AAOPlayerState* AOPS = GetPlayerState<AAOPlayerState>();
		//确保并等待PawnExtComp的初始化状态提前于当前this的状态
		return AOPS && Manager->HasFeatureReachedInitState(Pawn,UAOExtPawnComponent::NAME_ActorFeatureName,AOGameplayTags::InitState_DataInitialized);
	}
	else if (CurrentState == AOGameplayTags::InitState_DataInitialized && DesiredState == AOGameplayTags::InitState_GameplayReady)
	{
		return true;
	}
	return false;
}

//若状态能够推进，则执行该函数
void UAOHeroComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	IGameFrameworkInitStateInterface::HandleChangeInitState(Manager, CurrentState, DesiredState);
	if (CurrentState == AOGameplayTags::InitState_DataAvailable && DesiredState == AOGameplayTags::InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		AAOPlayerController* AOPC = GetController<AAOPlayerController>();
		AAOCharacter* CurrentCharacter = Cast<AAOCharacter>(Pawn);
		if (!ensure(Pawn && AOPC))
		{
			return;
		}

		if (UAOExtPawnComponent* ExtPawn = UAOExtPawnComponent::FindAOExtPawnComponent(Pawn))
		{
			ExtPawn->InitializeAbilitySystem(CurrentCharacter->GetSourceASC(),AOPC);
		}
	}
}

//当PawnExt的状态发生改变的时候会通过接口调用这个函数
void UAOHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UAOExtPawnComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == AOGameplayTags::InitState_DataInitialized)
		{
			//当PawnExtComp进入DataInitialized的时候继续推进this的下一个状态
			CheckDefaultInitialization();
		}
	}
}

void UAOHeroComponent::OnRegister()
{
	Super::OnRegister();
	RegisterInitStateFeature();  //注册初始化状态链
}

void UAOHeroComponent::BeginPlay()
{
	Super::BeginPlay();

	// 监听PawnExtComp的状态变化，会调用this的OnActorInitStateChanged函数
	BindOnActorInitStateChanged(UAOExtPawnComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// Notifies that we are done spawning, then try the rest of initialization
	ensure(TryToChangeInitState(AOGameplayTags::InitState_Spawned));
	
	CheckDefaultInitialization();
}

void UAOHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();
	Super::EndPlay(EndPlayReason);
}

void UAOHeroComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = { AOGameplayTags::InitState_Spawned, AOGameplayTags::InitState_DataAvailable,
		AOGameplayTags::InitState_DataInitialized, AOGameplayTags::InitState_GameplayReady };

	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}