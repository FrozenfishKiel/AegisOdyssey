// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularAIController.h"
#include "AegisOdyssey/Character/Enemies/Interface/AOEnemyPercepInterface.h"
#include "AAOAIPlayerBotController.generated.h"

class AActor;
class APlayerState;
class UAIPerceptionComponent;
class UObject;

UENUM(BlueprintType)
enum class EAIBotType : uint8
{
	Permanent UMETA(DisplayName = "Permanent AI"),
	Temporary UMETA(DisplayName = "Temporary AI"),
};

UCLASS(Blueprintable)
class AEGISODYSSEY_API AAOAIPlayerBotController : public AModularAIController
{
	GENERATED_BODY()

public:
	AAOAIPlayerBotController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void ServerRestartController();

	UFUNCTION(BlueprintCallable, Category = "AO AI Player Controller")
	void UpdateTeamAttitude(UAIPerceptionComponent* AIPerception);

	ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const;

	EAIBotType GetBotType() const { return BotType; }
	void SetBotType(EAIBotType InBotType) { BotType = InBotType; }

	UFUNCTION(BlueprintCallable, Category = "AO AI Player Controller")
	void DestroyAI();

	UFUNCTION(BlueprintCallable, Category = "AO AI Target")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

	UFUNCTION(BlueprintCallable, Category = "AO AI Target")
	void SetCurrentTarget(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "AO AI Target")
	float GetDistanceToCurrentTarget() const;

	UFUNCTION(BlueprintPure, Category = "AO AI Patrol")
	bool HasPatrolAnchorLocation() const { return bHasPatrolAnchorLocation; }

	UFUNCTION(BlueprintPure, Category = "AO AI Patrol")
	FVector GetPatrolAnchorLocation() const { return PatrolAnchorLocation; }

	UFUNCTION(BlueprintCallable, Category = "AO AI Patrol")
	void SetPatrolAnchorLocation(const FVector& NewPatrolAnchorLocation);

	UFUNCTION(BlueprintCallable, Category = "AO AI Patrol")
	void ResetPatrolAnchorLocationToPawn();

	UFUNCTION(BlueprintPure, Category = "AO AI Patrol")
	bool HasPatrolTargetLocation() const { return bHasPatrolTargetLocation; }

	UFUNCTION(BlueprintPure, Category = "AO AI Patrol")
	FVector GetPatrolTargetLocation() const { return PatrolTargetLocation; }

	UFUNCTION(BlueprintCallable, Category = "AO AI Patrol")
	void SetPatrolTargetLocation(const FVector& NewPatrolTargetLocation);

	UFUNCTION(BlueprintCallable, Category = "AO AI Patrol")
	void ClearPatrolTargetLocation();

protected:
	virtual void OnPlayerStateChanged();
	void BroadcastOnPlayerStateChanged();

	UFUNCTION()
	void OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	UFUNCTION()
	void OnPawnDeath(AActor* DestroyedActor);

	void HandleBotDeath();
	void DelayedDestroy();

protected:
	virtual void InitPlayerState() override;
	virtual void CleanupPlayerState() override;
	virtual void OnRep_PlayerState() override;
	virtual void OnUnPossess() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY()
	TObjectPtr<APlayerState> LastSeenPlayerState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AO AI Config", meta = (AllowPrivateAccess = "true"))
	EAIBotType BotType = EAIBotType::Temporary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AO AI Config", meta = (AllowPrivateAccess = "true", EditCondition = "BotType == EAIBotType::Permanent"))
	float RespawnDelay = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AO AI Config", meta = (AllowPrivateAccess = "true", EditCondition = "BotType == EAIBotType::Temporary"))
	float DestroyDelay = 3.0f;

	FTimerHandle DeathHandle_Timer;
	bool bDeathHandled = false;

	UPROPERTY(BlueprintReadWrite, Category = "AO AI Target", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<AActor> CurrentTarget;

	// 巡逻锚点用于给 Patrol EQS 提供一个稳定中心，避免脱战后每次都围绕当前瞬时位置乱飘。
	UPROPERTY(BlueprintReadOnly, Category = "AO AI Patrol", Transient, meta = (AllowPrivateAccess = "true"))
	FVector PatrolAnchorLocation = FVector::ZeroVector;

	// 用显式标记区分“合法零向量”和“当前还没有初始化巡逻锚点”这两种情况。
	UPROPERTY(BlueprintReadOnly, Category = "AO AI Patrol", Transient, meta = (AllowPrivateAccess = "true"))
	bool bHasPatrolAnchorLocation = false;

	// 这是“当前这一轮巡逻/走位真正要去的点”，用于跨兄弟状态共享 EQS 结果。
	UPROPERTY(BlueprintReadOnly, Category = "AO AI Patrol", Transient, meta = (AllowPrivateAccess = "true"))
	FVector PatrolTargetLocation = FVector::ZeroVector;

	// 用显式标记区分“没有缓存巡逻目标点”和“缓存点刚好是零向量”。
	UPROPERTY(BlueprintReadOnly, Category = "AO AI Patrol", Transient, meta = (AllowPrivateAccess = "true"))
	bool bHasPatrolTargetLocation = false;
};
