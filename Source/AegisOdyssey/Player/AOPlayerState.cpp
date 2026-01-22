// Fill out your copyright notice in the Description page of Project Settings.


#include "AOPlayerState.h"
#include "Components/GameFrameworkComponentManager.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "AegisOdyssey/Character/AOPawnData.h"
#include "AegisOdyssey/GameModes/AOExperienceManagerComponent.h"
#include "AegisOdyssey/GameModes/AOGameMode.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOPlayerState)

const FName AAOPlayerState::NAME_AOAbilityReady("AOAbilitiesReady");


AAOPlayerController* AAOPlayerState::GetAOPlayerController() const
{
	return Cast<AAOPlayerController>(GetOwner());
}

void AAOPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UWorld* World = GetWorld(); //获取当前世界
	if (World && World->IsGameWorld() && World->GetNetMode() != NM_Client)
	{
		AGameStateBase* GameState = GetWorld()->GetGameState();
		check(GameState);

		UAOExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UAOExperienceManagerComponent>();
		check(ExperienceComponent);
		ExperienceComponent->CallRegister_OnExperienceLoaded(FOnExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	}
}

void AAOPlayerState::ClientInitialize(class AController* C)
{
	Super::ClientInitialize(C);

	if (UAOExtPawnComponent* PawnExtComp = UAOExtPawnComponent::FindAOExtPawnComponent(GetPawn()))
	{
		PawnExtComp->CheckDefaultInitialization();
	}
}

void AAOPlayerState::OnExperienceLoaded(const UAOExperienceDefinition* ExperienceDefinition)
{
	if (AAOGameMode* AOGameMode = GetWorld()->GetAuthGameMode<AAOGameMode>())
	{
		if (const UAOPawnData* NewPawnData = AOGameMode->GetPawnDataForController(GetOwningController()))
		{
			SetPawnData(NewPawnData);
		}
	}
}


void AAOPlayerState::SetPawnData(const UAOPawnData* InPawnData)
{
	check(InPawnData);

	if (GetLocalRole() != ROLE_Authority)
	{
		return;  //确保这一过程在服务器执行
	}

	if (PawnData)
	{
		return;  //已经设置过PawnData
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass , PawnData , this);
	
	PawnData = InPawnData;

	for (const UAOAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_AOAbilityReady);

	ForceNetUpdate();
}

void AAOPlayerState::OnRep_PawnData()
{
	
}

void AAOPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;

	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);

	SharedParams.Condition = ELifetimeCondition::COND_SkipOwner;
}