// Fill out your copyright notice in the Description page of Project Settings.


#include "AOPlayerSpawningManagerComponent.h"

#include "AOPlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "EngineUtils.h"
#include "Engine/PlayerStartPIE.h"
#include "AOPlayerState.h"

UAOPlayerSpawningManagerComponent::UAOPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(false);
	bAutoRegister = true;
	bAutoActivate = true;
	bWantsInitializeComponent = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}


AActor* UAOPlayerSpawningManagerComponent::ChoosePlayerStart(AController* Player)
{
	if (Player)
	{
#if WITH_EDITOR
		if (AAOPlayerStart* PlayerStart = FindPlayerFromHereStart(Player))
		{
			return PlayerStart;
		}
#endif

		TArray<AAOPlayerStart*> PlayerStartPoints;  //遍历缓存的所有可能的出生点

		for (auto StartIt = CachedPlayerStarts.CreateIterator(); StartIt; ++StartIt)
		{
			if (AAOPlayerStart* Start = (*StartIt).Get())
			{
				PlayerStartPoints.Add(Start);
			}
			else
			{
				StartIt.RemoveCurrent();  //清理不为PlayerStart的缓存
			}
		}

		if (APlayerState* PlayerState = Player->GetPlayerState<AAOPlayerState>())
		{
			if (PlayerState->IsOnlyASpectator())  //当前玩家是否处于观战状态
			{
				if (!PlayerStartPoints.IsEmpty())
				{
					//为处于观战状态的玩家生成一个随机的出生点
					return PlayerStartPoints[FMath::RandRange(0, PlayerStartPoints.Num() - 1)];  //返回当前所有出生点的一个随机出生点
				}
				return nullptr;
			}
		}

		AActor* PlayerStart = OnChoosePlayerStart(Player , PlayerStartPoints);  //@TODO : 这个后面继承的时候可能会重写
		//AActor* PlayerStart = PlayerStartPoints[FMath::RandRange(0, PlayerStartPoints.Num() - 1)];  //随机选择一个出生点
		if (!PlayerStart)
		{
			PlayerStart = GetFirstRandomUnoccupiedPlayerStart(Player , PlayerStartPoints);
		}

		if (AAOPlayerStart* AOPlayerStart = Cast<AAOPlayerStart>(PlayerStart))
		{
			AOPlayerStart->TryClaim(Player);  //直接在当前出生点上生成角色
		}
		return PlayerStart;
	}
	return nullptr;
}

//尝试查询一个未被占用的出生点
AAOPlayerStart* UAOPlayerSpawningManagerComponent::GetFirstRandomUnoccupiedPlayerStart(AController* Controller,
	const TArray<AAOPlayerStart*>& FoundStartPoints) const
{
	//从所有的出生点中获取
	if (Controller)
	{
		TArray<AAOPlayerStart*> UnOccupiedStartPoints;  //附近没有一个玩家的出生点
		TArray<AAOPlayerStart*> OccupiedStartPoints;  //有几个但不多的玩家的出生点


		//遍历当前世界所有找到的已保存的出生点，查询当前它们的状态
		for (AAOPlayerStart* Start : FoundStartPoints)
		{
			EAOPlayerStartLocationOccupancy StartLocState = Start->GetLocationOccupancy(Controller);

			switch (StartLocState)
			{
			case EAOPlayerStartLocationOccupancy::Empty:
				UnOccupiedStartPoints.Add(Start);
				break;
			case EAOPlayerStartLocationOccupancy::Partial:
				OccupiedStartPoints.Add(Start);
				break;
			default:
				break;
			}
		}

		if (UnOccupiedStartPoints.Num() > 0)
		{
			return UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
		}
		else if (OccupiedStartPoints.Num() > 0)
		{
			return OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
		}
	}
	return nullptr;
}

void UAOPlayerSpawningManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	//注册当Level加载的时候会触发的委托
	FWorldDelegates::LevelAddedToWorld.AddUObject(this,&ThisClass::OnLevelAdded);

	//注册当Actor生成时会触发的委托
	UWorld* World = GetWorld();
	World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this,&UAOPlayerSpawningManagerComponent::HandleOnActorSpawned));

	//遍历所有的PlayerStart Actor ， 然后缓存到
	for (TActorIterator<AAOPlayerStart> It(World) ; It; ++It)
	{
		if (AAOPlayerStart* PlayerStart = *It)
		{
			CachedPlayerStarts.Add(PlayerStart);
		}
	}
}

void UAOPlayerSpawningManagerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

//当开始加载Level的时候
void UAOPlayerSpawningManagerComponent::OnLevelAdded(ULevel* InLevel, UWorld* InWorld)
{
	if (InWorld == GetWorld())
	{
		//遍历当前世界的所有Actor
		for (AActor* Play : InLevel->Actors)
		{
			//检查是否是玩家出生点Actor
			if (AAOPlayerStart* PlayerStart = Cast<AAOPlayerStart>(Play))
			{
				//如果在初始化的时候缓存容器里没有该PlayerStart则添加进去
				ensure(!CachedPlayerStarts.Contains(PlayerStart));
				CachedPlayerStarts.Add(PlayerStart);
			}
		}
	}
}

void UAOPlayerSpawningManagerComponent::HandleOnActorSpawned(AActor* SpawnedActor)
{
	//检查当前生成的Actor是否是PlayerStart
	if (AAOPlayerStart* PlayerStart = Cast<AAOPlayerStart>(SpawnedActor))
	{
		CachedPlayerStarts.Add(PlayerStart);
	}
}

#if WITH_EDITOR
AAOPlayerStart* UAOPlayerSpawningManagerComponent::FindPlayerFromHereStart(AController* Player)
{
	if (Player->IsA<APlayerController>())
	{
		if (UWorld* World = GetWorld())
		{
			for (TActorIterator<AAOPlayerStart> It(World) ; It; ++It)
			{
				if (AAOPlayerStart* PlayerStart = *It)
				{
					if (PlayerStart->IsA<APlayerStartPIE>())
					{
						return PlayerStart;
					}
				}
			}
		}
	}
	return nullptr;
}
#endif


bool UAOPlayerSpawningManagerComponent::ControllerCanRestart(AController* Player)
{
	bool bCanRestart = true;
	//未来需要添加的重生的逻辑
	return bCanRestart;
}

void UAOPlayerSpawningManagerComponent::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	
}
