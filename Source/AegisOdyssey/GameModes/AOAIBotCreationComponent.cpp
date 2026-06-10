// Fill out your copyright notice in the Description page of Project Settings.


#include "AOAIBotCreationComponent.h"

#include "AOExperienceManagerComponent.h"
#include "AOGameMode.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOAIBotCreationComponent)

UAOAIBotCreationComponent::UAOAIBotCreationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAOAIBotCreationComponent::BeginPlay()
{
	Super::BeginPlay();

	// 监听Experience加载完成事件
	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (GameState)
	{
		UAOExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<UAOExperienceManagerComponent>();
		if (ExperienceComponent)
		{
			ExperienceComponent->CallRegister_OnExperienceLoaded(FOnExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
		}
	}
}

void UAOAIBotCreationComponent::OnExperienceLoaded(const UAOExperienceDefinition* Experience)
{
#if WITH_SERVER_CODE
	if (HasAuthority())
	{
		ServerCreateBots();  //通过服务器创建AI 
	}
#endif
}

#if WITH_SERVER_CODE

void UAOAIBotCreationComponent::ServerCreateBots_Implementation()
{
	// 检查BotController类是否设置
	if (BotControllerClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AOAIBotCreationComponent: BotControllerClass is not set!"));
		return;
	}

	// 初始化Bot名称列表
	RemainingBotNames = RandomBotNames;

	// 计算有效的Bot数量
	int32 EffectiveBotCount = NumBotsToCreate;

	// 允许URL参数覆盖Bot数量
	if (AGameModeBase* GameModeBase = GetWorld()->GetAuthGameMode())
	{
		EffectiveBotCount = UGameplayStatics::GetIntOption(GameModeBase->OptionsString, TEXT("NumBots"), EffectiveBotCount);
	}

	// 创建Bots
	for (int32 Count = 0; Count < EffectiveBotCount; ++Count)
	{
		SpawnOneBot();
	}
}

#endif

FString UAOAIBotCreationComponent::CreateBotName(int32 PlayerIndex)
{
	FString Result;
	if (RemainingBotNames.Num() > 0)
	{
		// 从剩余名称中随机选择一个
		const int32 NameIndex = FMath::RandRange(0, RemainingBotNames.Num() - 1);
		Result = RemainingBotNames[NameIndex];
		RemainingBotNames.RemoveAtSwap(NameIndex);
	}
	else
	{
		// 如果没有预设名称，生成默认名称
		PlayerIndex = FMath::RandRange(260, 260 + 100);
		Result = FString::Printf(TEXT("AIBot_%d"), PlayerIndex);
	}
	return Result;
}

// 游戏体验加载完毕会开始生成AI控制器
void UAOAIBotCreationComponent::SpawnOneBot()
{
	// 使用默认AI类型生成
	SpawnBotWithType(DefaultBotType);
}

AAIController* UAOAIBotCreationComponent::SpawnBotWithType(EAIBotType BotType)
{
	// 生成AIController
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.OverrideLevel = GetComponentLevel();
	SpawnInfo.ObjectFlags |= RF_Transient;

	// 生成控制器
	AAIController* NewController = GetWorld()->SpawnActor<AAIController>(
		BotControllerClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnInfo
	);

	if (NewController != nullptr)
	{
		AAOGameMode* GameMode = Cast<AAOGameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode)
		{
			// 如果是AOAIPlayerBotController，设置AI类型
			if (AAOAIPlayerBotController* AOBotController = Cast<AAOAIPlayerBotController>(NewController))
			{
				AOBotController->SetBotType(BotType);
				UE_LOG(LogTemp, Log, TEXT("AOAIBotCreationComponent: Spawned bot [%s] with type [%s]"), 
					*NewController->GetName(), 
					BotType == EAIBotType::Permanent ? TEXT("Permanent") : TEXT("Temporary"));
			}

			// 设置Bot名称
			if (NewController->PlayerState != nullptr)
			{
				NewController->PlayerState->SetPlayerName(CreateBotName(NewController->PlayerState->GetPlayerId()));
			}

			// 初始化Bot（GenericPlayerInitialization会触发OnAOGameModePlayerInitialized委托）
			GameMode->GenericPlayerInitialization(NewController);

			// 重启玩家（生成Pawn）
			GameMode->RestartPlayer(NewController);

			// 触发Pawn初始化检查
			if (NewController->GetPawn() != nullptr)
			{
				if (UAOExtPawnComponent* PawnExtComponent = NewController->GetPawn()->FindComponentByClass<UAOExtPawnComponent>())
				{
					PawnExtComponent->CheckDefaultInitialization();
				}
			}

			// 添加到已生成列表
			SpawnedBotList.Add(NewController);

			UE_LOG(LogTemp, Log, TEXT("AOAIBotCreationComponent: Successfully spawned bot [%s]"), *NewController->GetName());
		}
	}

	return NewController;
}

void UAOAIBotCreationComponent::RemoveOneBot()
{
	if (SpawnedBotList.Num() > 0)
	{
		// 随机移除一个Bot
		const int32 BotToRemoveIndex = FMath::RandRange(0, SpawnedBotList.Num() - 1);
		AAIController* BotToRemove = SpawnedBotList[BotToRemoveIndex];

		if (BotToRemove)
		{
			// 销毁Pawn
			if (APawn* Pawn = BotToRemove->GetPawn())
			{
				Pawn->Destroy();
			}

			// 销毁Controller
			BotToRemove->Destroy();
		}

		SpawnedBotList.RemoveAtSwap(BotToRemoveIndex);
	}
}
