// Fill out your copyright notice in the Description page of Project Settings.


#include "AOPlayerStart.h"

#include "GameFramework/GameModeBase.h"

AAOPlayerStart::AAOPlayerStart(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	
}

EAOPlayerStartLocationOccupancy AAOPlayerStart::GetLocationOccupancy(AController* const ControllerPawnToFit) const
{
	UWorld* const World = GetWorld();  //获取当前世界
	// 确保当前处于服务器运行
	if (HasAuthority() && World)
	{
		//获取游戏模式
		if (AGameModeBase* AuthGameMode = World->GetAuthGameMode())
		{
			//从当前玩家中获取它的Default Pawn
			TSubclassOf<APawn> PawnClass = AuthGameMode->GetDefaultPawnClassForController(ControllerPawnToFit);
			const APawn* const PawnToFit = PawnClass ? GetDefault<APawn>(PawnClass) : nullptr;

			FVector ActorLocation = GetActorLocation();  //获取当前出生点的位置
			const FRotator ActorRotation = GetActorRotation();

			// 在世界上模拟放置一个Pawn（并不会实际生成） 用于检查环境（比如是否碰撞），如果环境安全则返回“空”
			if (!World->EncroachingBlockingGeometry(PawnToFit , ActorLocation , ActorRotation , nullptr))
			{
				return EAOPlayerStartLocationOccupancy::Empty;
			}
			// 与上一个函数在于，它会尝试微调位置，假设当前位置有人，但是附近一点点没人，那也可以生成，此时返回“部分”
			else if (World->FindTeleportSpot(PawnToFit , ActorLocation , ActorRotation))
			{
				return EAOPlayerStartLocationOccupancy::Partial;
			}
		}
	}
	return EAOPlayerStartLocationOccupancy::Full;
}

bool AAOPlayerStart::IsClaimed() const
{
	return ClaimingController != nullptr;
}

//定时器检查当前需要生成的玩家是否已离开
bool AAOPlayerStart::TryClaim(AController* OccupyingController)
{
	if (OccupyingController != nullptr && !IsClaimed())
	{
		ClaimingController = OccupyingController;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ExpirationTimerHandle , FTimerDelegate::CreateUObject(this , &AAOPlayerStart::CheckUnClaimed) , ExpirationCheckInterval , true);
		}
		return true;
	}
	return false;
}

void AAOPlayerStart::CheckUnClaimed()
{
	if (ClaimingController != nullptr && ClaimingController->GetPawn() != nullptr && GetLocationOccupancy(ClaimingController) != EAOPlayerStartLocationOccupancy::Empty)
	{
		ClaimingController = nullptr;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ExpirationTimerHandle);  //清空定时器
		}
	}
}
