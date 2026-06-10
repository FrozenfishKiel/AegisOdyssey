// Fill out your copyright notice in the Description page of Project Settings.


#include "AOPlayerState.h"

#include "GameFeaturesSubsystem.h"
#include "Components/GameFrameworkComponentManager.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "AegisOdyssey/Character/AOPawnData.h"
#include "GameFeatureAction.h"
#include "AegisOdyssey/GameModes/AOExperienceManagerComponent.h"
#include "AegisOdyssey/GameModes/AOGameMode.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Player/AOPlayerController.h"

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

	FGameFeatureActivatingContext Context;
	const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
	if (ExistingWorldContext)
	{
		Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
	}
	auto ActivationListOfActions = [&Context] (const TArray<UGameFeatureAction*>& InActions)
	{
		for (UGameFeatureAction* Action : InActions)
		{
			if (Action != nullptr)
			{
				Action->OnGameFeatureRegistering();
				Action->OnGameFeatureLoading();
				Action->OnGameFeatureActivating(Context);
			}
		}
	};

	ActivationListOfActions(PawnData->Actions);
	
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

// ========== 等级系统实现 ==========

int32 AAOPlayerState::GetCharacterLevel() const
{
	if (const AAOCharacter* Character = GetControlledCharacter())
	{
		return Character->GetCharacterLevel();
	}
	return 1;
}

int32 AAOPlayerState::GetCharacterXP() const
{
	if (const AAOCharacter* Character = GetControlledCharacter())
	{
		return Character->GetCharacterXP();
	}
	return 0;
}

int32 AAOPlayerState::GetAvailableAttributePoints() const
{
	if (const AAOCharacter* Character = GetControlledCharacter())
	{
		return Character->GetAvailableAttributePoints();
	}
	return 0;
}

void AAOPlayerState::SetCharacterLevel(int32 NewLevel)
{
	if (AAOCharacter* Character = GetControlledCharacter())
	{
		Character->SetCharacterLevel(NewLevel);
	}
}

void AAOPlayerState::AddToCharacterLevel(int32 DeltaLevel)
{
	if (AAOCharacter* Character = GetControlledCharacter())
	{
		Character->AddToCharacterLevel(DeltaLevel);
	}
}

void AAOPlayerState::SetCharacterXP(int32 NewXP)
{
	if (AAOCharacter* Character = GetControlledCharacter())
	{
		Character->SetCharacterXP(NewXP);
	}
}

void AAOPlayerState::AddToCharacterXP(int32 DeltaXP)
{
	if (AAOCharacter* Character = GetControlledCharacter())
	{
		Character->AddToCharacterXP(DeltaXP);
	}
}

void AAOPlayerState::SetAvailableAttributePoints(int32 NewPoints)
{
	if (AAOCharacter* Character = GetControlledCharacter())
	{
		Character->SetAvailableAttributePoints(NewPoints);
	}
}

void AAOPlayerState::AddToAvailableAttributePoints(int32 DeltaPoints)
{
	if (AAOCharacter* Character = GetControlledCharacter())
	{
		Character->AddToAvailableAttributePoints(DeltaPoints);
	}
}

AAOCharacter* AAOPlayerState::GetControlledCharacter() const
{
	if (APlayerController* PC = GetPlayerController())
	{
		return Cast<AAOCharacter>(PC->GetPawn());
	}
	return nullptr;
}

void AAOPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PC = GetPlayerController())
	{
		PC->OnPossessedPawnChanged.AddDynamic(this, &AAOPlayerState::OnPawnChanged);
		
		OnPawnChanged(PC->GetPawn(), nullptr);
	}
}

void AAOPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CurrentControlledCharacter)
	{
		UnbindCharacterDelegates(CurrentControlledCharacter);
	}
	
	Super::EndPlay(EndPlayReason);
}

void AAOPlayerState::OnPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (AAOCharacter* OldCharacter = Cast<AAOCharacter>(OldPawn))
	{
		UnbindCharacterDelegates(OldCharacter);
	}
	
	CurrentControlledCharacter = Cast<AAOCharacter>(NewPawn);
	
	if (CurrentControlledCharacter)
	{
		BindCharacterDelegates(CurrentControlledCharacter);
	}
}

void AAOPlayerState::BindCharacterDelegates(AAOCharacter* Character)
{
	if (!Character) return;
	
	Character->OnCharacterLevelChangedDelegate.AddLambda(
		[this](int32 OldLevel, int32 NewLevel)
		{
			OnLevelChangedDelegate.Broadcast(OldLevel, NewLevel);
		}
	);
	
	Character->OnCharacterXPChangedDelegate.AddLambda(
		[this](int32 OldXP, int32 NewXP)
		{
			OnXPChangedDelegate.Broadcast(OldXP, NewXP);
		}
	);
	
	Character->OnAttributePointsChangedDelegate.AddLambda(
		[this](int32 OldPoints, int32 NewPoints)
		{
			OnAttributePointsChangedDelegate.Broadcast(OldPoints, NewPoints);
		}
	);
}

void AAOPlayerState::UnbindCharacterDelegates(AAOCharacter* Character)
{
	if (!Character) return;
	
	Character->OnCharacterLevelChangedDelegate.RemoveAll(this);
	Character->OnCharacterXPChangedDelegate.RemoveAll(this);
	Character->OnAttributePointsChangedDelegate.RemoveAll(this);
}
