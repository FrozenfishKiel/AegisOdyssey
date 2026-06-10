// Fill out your copyright notice in the Description page of Project Settings.

#include "AAOAIPlayerBotController.h"

#include "AegisOdyssey/Character/AOExtPawnComponent.h"
#include "AegisOdyssey/Character/CombatInterface.h"
#include "AegisOdyssey/Character/Interfaces/AOBotInterface.h"
#include "AegisOdyssey/GameModes/AOGameMode.h"
#include "AegisOdyssey/Player/AOPlayerState.h"
#include "AegisOdyssey/StateTree/AOStateTreeComponentBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionComponent.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AAOAIPlayerBotController)

AAOAIPlayerBotController::AAOAIPlayerBotController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsPlayerState = true;
	bStopAILogicOnUnposses = false;
}

void AAOAIPlayerBotController::OnPlayerStateChanged()
{
}

void AAOAIPlayerBotController::BroadcastOnPlayerStateChanged()
{
	OnPlayerStateChanged();
	LastSeenPlayerState = PlayerState;
}

void AAOAIPlayerBotController::OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
}

void AAOAIPlayerBotController::InitPlayerState()
{
	Super::InitPlayerState();
	BroadcastOnPlayerStateChanged();
}

void AAOAIPlayerBotController::CleanupPlayerState()
{
	Super::CleanupPlayerState();
	BroadcastOnPlayerStateChanged();
}

void AAOAIPlayerBotController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BroadcastOnPlayerStateChanged();
}

void AAOAIPlayerBotController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	bDeathHandled = false;

	if (InPawn == nullptr)
	{
		return;
	}

	IAOBotInterface* EnemyPawnInterface = Cast<IAOBotInterface>(InPawn);
	if (EnemyPawnInterface == nullptr)
	{
		return;
	}

	if (!bHasPatrolAnchorLocation)
	{
		// 只在第一次接管时记录默认锚点，这样脱战、重生后仍能回到稳定的 Home 区域。
		PatrolAnchorLocation = InPawn->GetActorLocation();
		bHasPatrolAnchorLocation = true;
	}

	// Override the player-facing rotation setup with AI-friendly settings for possessed bots only.
	if (ACharacter* PossessedCharacter = Cast<ACharacter>(InPawn))
	{
		PossessedCharacter->bUseControllerRotationYaw = false;

		if (UCharacterMovementComponent* MoveComp = PossessedCharacter->GetCharacterMovement())
		{
			MoveComp->bUseControllerDesiredRotation = true;
			MoveComp->bOrientRotationToMovement = false;
		}
	}

	InPawn->OnDestroyed.AddDynamic(this, &AAOAIPlayerBotController::OnPawnDeath);

	if (AAOPlayerState* SourcePS = GetPlayerState<AAOPlayerState>())
	{
		SourcePS->SetPawnData(EnemyPawnInterface->GetPawnData());
	}

	if (UAOExtPawnComponent* ExtPawnComponent = InPawn->FindComponentByClass<UAOExtPawnComponent>())
	{
		if (!ExtPawnComponent->GetPawnData<UAOPawnData>())
		{
			ExtPawnComponent->SetDataPawn(EnemyPawnInterface->GetPawnData());
			ExtPawnComponent->CheckDefaultInitialization();
		}
	}

	// This StateTree component is added inside SetPawnData() via GameFeature AddComponent,
	// so it misses the earlier PossessedBy-driven restart on the pawn.
	TArray<UActorComponent*> StateTreeComponents;
	InPawn->GetComponents(UAOStateTreeComponentBase::StaticClass(), StateTreeComponents);
	for (UActorComponent* Component : StateTreeComponents)
	{
		UAOStateTreeComponentBase* StateTreeComponent = Cast<UAOStateTreeComponentBase>(Component);
		if (StateTreeComponent != nullptr && StateTreeComponent->GetStateTreeAsset() != nullptr)
		{
			StateTreeComponent->RestartLogic();
		}
	}
}

void AAOAIPlayerBotController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AAOAIPlayerBotController::OnPawnDeath(AActor* DestroyedActor)
{
	if (bDeathHandled)
	{
		return;
	}

	bDeathHandled = true;
	GetWorldTimerManager().ClearTimer(DeathHandle_Timer);
	HandleBotDeath();
}

void AAOAIPlayerBotController::HandleBotDeath()
{
	if (BotType == EAIBotType::Temporary)
	{
		UE_LOG(LogTemp, Log, TEXT("Temporary AI [%s] died and will be destroyed in %.1f seconds"), *GetName(), DestroyDelay);
		GetWorldTimerManager().SetTimer(DeathHandle_Timer, this, &AAOAIPlayerBotController::DelayedDestroy, DestroyDelay);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Permanent AI [%s] died and will respawn in %.1f seconds"), *GetName(), RespawnDelay);

		if (GetPawn())
		{
			UnPossess();
		}

		GetWorldTimerManager().SetTimer(DeathHandle_Timer, this, &AAOAIPlayerBotController::ServerRestartController, RespawnDelay);
	}
}

void AAOAIPlayerBotController::DelayedDestroy()
{
	UE_LOG(LogTemp, Log, TEXT("Destroying temporary AI [%s]"), *GetName());

	if (GetPawn())
	{
		GetPawn()->Destroy();
	}

	Destroy();
}

void AAOAIPlayerBotController::DestroyAI()
{
	HandleBotDeath();
}

void AAOAIPlayerBotController::ServerRestartController()
{
	if (GetNetMode() == NM_Client)
	{
		return;
	}

	if (BotType != EAIBotType::Permanent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Temporary AI [%s] cannot respawn"), *GetName());
		return;
	}

	ensure((GetPawn() == nullptr) && IsInState(NAME_Inactive));
	if (IsInState(NAME_Inactive) || IsInState(NAME_Spectating))
	{
		AAOGameMode* const GameMode = GetWorld()->GetAuthGameMode<AAOGameMode>();

		if ((GameMode == nullptr) || !GameMode->ControllerCanRestart(this))
		{
			return;
		}

		if (GetPawn() != nullptr)
		{
			UnPossess();
		}

		ResetIgnoreInputFlags();
		bDeathHandled = false;

		GameMode->RestartPlayer(this);
	}
}

ETeamAttitude::Type AAOAIPlayerBotController::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (const APawn* OtherPawn = Cast<APawn>(&Other))
	{
		AController* OtherController = OtherPawn->GetController();
		if (OtherController)
		{
			if (OtherController->IsA(AAOAIPlayerBotController::StaticClass()))
			{
				return ETeamAttitude::Friendly;
			}

			return ETeamAttitude::Hostile;
		}
	}

	return ETeamAttitude::Neutral;
}

void AAOAIPlayerBotController::UpdateTeamAttitude(UAIPerceptionComponent* AIPerception)
{
	if (AIPerception)
	{
		AIPerception->RequestStimuliListenerUpdate();
	}
}

void AAOAIPlayerBotController::SetCurrentTarget(AActor* NewTarget)
{
	CurrentTarget = NewTarget;

	if (NewTarget != nullptr)
	{
		SetFocus(NewTarget, EAIFocusPriority::Gameplay);
	}
	else
	{
		ClearFocus(EAIFocusPriority::Gameplay);
	}
}

float AAOAIPlayerBotController::GetDistanceToCurrentTarget() const
{
	if (CurrentTarget.IsValid() && GetPawn())
	{
		return FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
	}

	return 0.0f;
}

void AAOAIPlayerBotController::SetPatrolAnchorLocation(const FVector& NewPatrolAnchorLocation)
{
	PatrolAnchorLocation = NewPatrolAnchorLocation;
	bHasPatrolAnchorLocation = true;
}

void AAOAIPlayerBotController::ResetPatrolAnchorLocationToPawn()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		PatrolAnchorLocation = ControlledPawn->GetActorLocation();
		bHasPatrolAnchorLocation = true;
	}
}

void AAOAIPlayerBotController::SetPatrolTargetLocation(const FVector& NewPatrolTargetLocation)
{
	PatrolTargetLocation = NewPatrolTargetLocation;
	bHasPatrolTargetLocation = true;
}

void AAOAIPlayerBotController::ClearPatrolTargetLocation()
{
	PatrolTargetLocation = FVector::ZeroVector;
	bHasPatrolTargetLocation = false;
}

void AAOAIPlayerBotController::OnUnPossess()
{
	ClearFocus(EAIFocusPriority::Gameplay);
	CurrentTarget = nullptr;
	ClearPatrolTargetLocation();

	if (APawn* PawnBeingUnpossessed = GetPawn())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PlayerState))
		{
			if (ASC->GetAvatarActor() == PawnBeingUnpossessed)
			{
				ASC->SetAvatarActor(nullptr);
			}
		}
	}

	Super::OnUnPossess();
}
