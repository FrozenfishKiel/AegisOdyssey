// Fill out your copyright notice in the Description page of Project Settings.


#include "AOGameMode.h"

#include "AOExperienceManagerComponent.h"
#include "AOGameState.h"
#include "AOWorldSettings.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "AegisOdyssey/Player/AAOAIPlayerBotController.h"
#include "AegisOdyssey/Player/AOPlayerController.h"
#include "AegisOdyssey/Player/AOPlayerSpawningManagerComponent.h"
#include "AegisOdyssey/Player/AOPlayerState.h"
#include "AegisOdyssey/UI/AOHUD.h"
#include "GameFramework/GameSession.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOGameMode)


AAOGameMode::AAOGameMode(const FObjectInitializer& ObjectInitializer)
{
	GameStateClass = AAOGameState::StaticClass();
	GameSessionClass = AGameSession::StaticClass();
	PlayerControllerClass = AAOPlayerController::StaticClass();
	PlayerStateClass = AAOPlayerState::StaticClass();
	DefaultPawnClass = AAOCharacter::StaticClass();
	HUDClass = AAOHUD::StaticClass();
}


void AAOGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	GetWorld()->GetTimerManager().SetTimerForNextTick(this,&ThisClass::HandleGameInitialize);  //在下一帧执行

	// 禁用状态树运行时验证
	static IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("StateTree.RuntimeValidation.Context"));
	if (CVar)
	{
		CVar->Set(0);
	}
}

void AAOGameMode::HandleGameInitialize()
{
	HandleMatchAssignmentIfNotExpectingOne();
}

void AAOGameMode::HandleMatchAssignmentIfNotExpectingOne()
{
	FPrimaryAssetId ExperienceId;
	AAOWorldSettings* TypedWorldSettings = Cast<AAOWorldSettings>(GetWorld()->GetWorldSettings());

	if (TypedWorldSettings)
	{
		ExperienceId = TypedWorldSettings->GetDefaultGameplayExperience();  //获取当前world的GameplayDefinition（也就是ExperienceID）
	}
	if (ExperienceId.IsValid())
	{
		UAOExperienceManagerComponent* AOGameStateComponent = GameState->FindComponentByClass<UAOExperienceManagerComponent>();
		check(AOGameStateComponent);
		AOGameStateComponent->SetCurrentExperience(ExperienceId);
	}
	else
	{
		UAOExperienceManagerComponent* AOGameStateComponent = GameState->FindComponentByClass<UAOExperienceManagerComponent>();
		check(AOGameStateComponent);
		AOGameStateComponent->SetCurrentExperience(TypedWorldSettings->GetFeatureDefinitionClass());
	}
}


UClass* AAOGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const UAOPawnData* PawnData = GetPawnDataForController(InController))
	{
		if (PawnData->PawnClass)
		{
			return PawnData->PawnClass;
		}
	}
	
	
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}


#include "AegisOdyssey/Character/Interfaces/AOBotInterface.h"
#include "AegisOdyssey/Player/AAOAIPlayerBotController.h"

APawn* AAOGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.bDeferConstruction = true;

	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		if (APawn* Spawned = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo))
		{
			const bool bIsLocallyControlled = Spawned->IsLocallyControlled();
			const bool bIsBot = Spawned->IsBotControlled();
			if (UAOExtPawnComponent* PawnExtComp = UAOExtPawnComponent::FindAOExtPawnComponent(Spawned))
			{
				if (bIsLocallyControlled && !bIsBot)
				{
					if (const UAOPawnData* PawnData = GetPawnDataForController(NewPlayer))
					{
						PawnExtComp->SetDataPawn(PawnData);
					}
				}
				else if (bIsLocallyControlled && bIsBot)
				{
					if (IAOBotInterface* BotInterface = Cast<IAOBotInterface>(Spawned))
					{
						PawnExtComp->SetDataPawn(BotInterface->GetPawnData());
					}
				}
			}

			Spawned->FinishSpawning(SpawnTransform);
			return Spawned;
		}
	}

	return nullptr;
}


const UAOPawnData* AAOGameMode::GetPawnDataForController(const AController* InController) const
{
	//先尝试从已有的PlayerState中获取PawnData
	
	if (InController != nullptr)
	{
		if (const AAOPlayerState* AOPS = InController->GetPlayerState<AAOPlayerState>())
		{
			if (const UAOPawnData* PawnData = AOPS->GetPawnData<UAOPawnData>())
			{
				return PawnData;
			}
		}
	}
	//如果从PlayerState中拿不到PawnData，则尝试从游戏体验信息中获取默认的PawnData
	check(GameState);
	UAOExperienceManagerComponent* ExperienceManagerComponent = GameState->FindComponentByClass<UAOExperienceManagerComponent>();
	check(ExperienceManagerComponent);

	if (ExperienceManagerComponent->IsExperienceLoaded())
	{
		const UAOExperienceDefinition* Experience = ExperienceManagerComponent->GetCurrentExperienceCheck();
		if (Experience->DefaultPawnData)
		{
			return Experience->DefaultPawnData;
		}
	}

	//如果从PS中加载不到PawnData，就从资产管理器中加载
	
	return nullptr;

}

void AAOGameMode::InitGameState()
{
	Super::InitGameState();

	UAOExperienceManagerComponent* ExperienceManagerComponent = GameState->FindComponentByClass<UAOExperienceManagerComponent>();
	check(ExperienceManagerComponent);
	ExperienceManagerComponent->CallRegister_OnExperienceLoaded(FOnExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

bool AAOGameMode::PlayerCanRestart_Implementation(APlayerController* Player)
{
	return ControllerCanRestart(Player);
}

bool AAOGameMode::ControllerCanRestart(AController* Controller)
{
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (!Super::PlayerCanRestart_Implementation(PC))
		{
			return false;
		}
	}
	else
	{
		//PC已被GC
		if (Controller == nullptr || Controller->IsPendingKillPending())
		{
			return false;
		}
	}

	if (UAOPlayerSpawningManagerComponent* PlayerSpawningManagerComponent = GameState->FindComponentByClass<UAOPlayerSpawningManagerComponent>())
	{
		return PlayerSpawningManagerComponent->ControllerCanRestart(Controller);
	}

	return true;
}

void AAOGameMode::RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset)
{
	if (bForceReset && (Controller != nullptr))
	{
		Controller->Reset();
	}

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		// 通知服务器重新生成玩家
		GetWorldTimerManager().SetTimerForNextTick(PC,&APlayerController::ServerRestartPlayer_Implementation);
	}
	else if (AAOAIPlayerBotController* BotController = Cast<AAOAIPlayerBotController>(Controller))
	{
		// AI Bot重启
		GetWorldTimerManager().SetTimerForNextTick(BotController, &AAOAIPlayerBotController::ServerRestartController);
	}
}


AActor* AAOGameMode::ChoosePlayerStart_Implementation(AController* Player)
{

	if (UAOPlayerSpawningManagerComponent* PlayerSpawningManagerComponent = GameState->FindComponentByClass<UAOPlayerSpawningManagerComponent>())
	{
		return PlayerSpawningManagerComponent->ChoosePlayerStart(Player);
	}

	return Super::ChoosePlayerStart_Implementation(Player);

}

void AAOGameMode::OnExperienceLoaded(const UAOExperienceDefinition* CurrentExperience)
{
	if (CurrentExperience)
	{
		if (UWorld* World = GetWorld())
		{
			int32 Players = World->GetNumPlayerControllers();
			if (Players > 0)
			{
				for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
				{
					APlayerController* PC = Cast<APlayerController>(*Iterator);
					if (PC != nullptr && (PC->GetPawn() == nullptr))
					{
						if (PlayerCanRestart(PC))
						{
							RestartPlayer(PC);
						}
					}
				}
			}
		}
	}
}

bool AAOGameMode::IsExperienceLoaded() const
{
	check(GameState);

	UAOExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UAOExperienceManagerComponent>();
	check(ExperienceComponent);

	return ExperienceComponent->IsExperienceLoaded();
}

//玩家登录时处理新的PlayerController进入世界
void AAOGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	if (IsExperienceLoaded())
	{
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}

void AAOGameMode::FailedToRestartPlayer(AController* NewPlayer)
{
	//尝试重新生成玩家失败

	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		if (APlayerController* NewPC = Cast<APlayerController>(NewPlayer))
		{
			if (PlayerCanRestart(NewPC))
			{
				RequestPlayerRestartNextFrame(NewPlayer,false);
			}
			else
			{
				UE_LOG(LogTemp, Verbose, TEXT("服务器未找到控制器"), *GetPathNameSafe(NewPlayer));
			}
		}
		else
		{
			RequestPlayerRestartNextFrame(NewPlayer, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("服务器未找到当前的PawnData"), *GetPathNameSafe(NewPlayer));

	}
	
	Super::FailedToRestartPlayer(NewPlayer);
}

void AAOGameMode::GenericPlayerInitialization(AController* C)
{
	Super::GenericPlayerInitialization(C);
	
	OnAOGameModePlayerInitialized.Broadcast(this,C);
}

void AAOGameMode::InitializeHUDForPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::InitializeHUDForPlayer_Implementation(NewPlayer);
}

void AAOGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

void AAOGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	Super::FinishRestartPlayer(NewPlayer, StartRotation);

	if (UAOPlayerSpawningManagerComponent* PlayerSpawningComponent = GameState->FindComponentByClass<UAOPlayerSpawningManagerComponent>())
	{
		PlayerSpawningComponent->FinishRestartPlayer(NewPlayer, StartRotation);
	}
	Super::FinishRestartPlayer(NewPlayer, StartRotation);
}

bool AAOGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

bool AAOGameMode::UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage)
{
	return true;
}
