// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerStart.h"
#include "AOPlayerStart.generated.h"

/**
 * 
 */

enum class EAOPlayerStartLocationOccupancy
{
	Empty,
	Partial,
	Full
};
UCLASS()
class AEGISODYSSEY_API AAOPlayerStart : public APlayerStart
{
	GENERATED_BODY()
public:

	AAOPlayerStart(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	const FGameplayTagContainer& GetGameplayTags() {return StartPointTags;}

	EAOPlayerStartLocationOccupancy GetLocationOccupancy(AController* const ControllerPawnToFit) const;

	//检查出生点是否占用
	bool IsClaimed() const;

	// 如果this PlayerStart未占用 则在其位置生成玩家
	bool TryClaim(AController* OccupyingController);
protected:
	void CheckUnClaimed();  //查找未占用的出生点

	UPROPERTY(Transient)
	TObjectPtr<AController> ClaimingController = nullptr;
	
	/** Interval in which we'll check if this player start is not colliding with anyone anymore */
	UPROPERTY(EditDefaultsOnly, Category = "Player Start Claiming")
	float ExpirationCheckInterval = 1.f;  //每隔xx秒检查一次出生点状态

	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer StartPointTags;  //出生点的标记标签

	/** Handle to track expiration recurring timer */
	FTimerHandle ExpirationTimerHandle;  //定时器句柄，这个定时器是要定期检查出生点是否可靠的
};
