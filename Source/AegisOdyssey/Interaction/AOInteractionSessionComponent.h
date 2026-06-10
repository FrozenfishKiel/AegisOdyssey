// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AegisOdyssey/Interaction/Session/AOContainerInteractionSessionModel.h"
#include "Templates/Function.h"
#include "AOInteractionSessionComponent.generated.h"

class UCommonActivatableWidget;
class UAOInteractionSessionModel;
class UAOInventoryComponent;

USTRUCT()
struct FAOReplicatedInteractionSessionState
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHasActiveSession = false;

	UPROPERTY()
	TObjectPtr<AActor> InteractableActor = nullptr;

	UPROPERTY()
	TSubclassOf<UCommonActivatableWidget> SessionWidgetClass;

	UPROPERTY()
	TObjectPtr<UAOInventoryComponent> ContainerInventory = nullptr;

	UPROPERTY()
	TArray<FAOObservedInventorySlot> ContainerSlots;

};

struct FAOInteractableMutationRequest
{
	FAOInteractableMutationRequest() = default;

	FAOInteractableMutationRequest(FName InDebugName, TFunction<void()>&& InExecuteAction)
		: DebugName(InDebugName)
		, ExecuteAction(MoveTemp(InExecuteAction))
	{
	}

	FAOInteractableMutationRequest(FName InDebugName, TFunction<bool()>&& InValidateAction, TFunction<void()>&& InExecuteAction)
		: DebugName(InDebugName)
		, ValidateAction(MoveTemp(InValidateAction))
		, ExecuteAction(MoveTemp(InExecuteAction))
	{
	}

	bool IsValid() const
	{
		return ExecuteAction.operator bool();
	}

	bool CanExecute() const
	{
		return !ValidateAction || ValidateAction();
	}

	void Execute() const
	{
		if (ExecuteAction)
		{
			ExecuteAction();
		}
	}

	FName DebugName = NAME_None;
	TFunction<bool()> ValidateAction;
	TFunction<void()> ExecuteAction;
};

UCLASS(ClassGroup = ("AO"), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOInteractionSessionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentSessionChanged, UAOInteractionSessionModel*);

	UAOInteractionSessionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	void StartSession(UAOInteractionSessionModel* NewSessionModel);
	void CloseCurrentSession();

	UFUNCTION(BlueprintCallable, Category = "AO|Interaction")
	void RequestCloseCurrentSession();

	UFUNCTION(BlueprintCallable, Category = "AO|Interaction")
	void RequestAcquireCurrentInteractableOwner();

	UFUNCTION(BlueprintCallable, Category = "AO|Interaction")
	void RequestReleaseCurrentInteractableOwner();

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	bool HasCurrentInteractableMutationAuthority() const;

	bool SubmitCurrentInteractableMutation(FAOInteractableMutationRequest&& MutationRequest);
	bool ExecuteOrQueueCurrentInteractableMutation(TFunction<void()>&& MutationAction);
	bool ExecuteCurrentContainerMutationRequest(const FAOContainerSessionMutationRequest& MutationRequest);

	void SyncCurrentSessionToReplication();

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	bool HasActiveSession() const;

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	AActor* GetCurrentInteractableActor() const;

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	UAOInteractionSessionModel* GetCurrentSessionModel() const { return CurrentSessionModel; }

	FOnCurrentSessionChanged& GetOnCurrentSessionChanged() { return OnCurrentSessionChanged; }

	UFUNCTION(BlueprintPure, Category = "AO|Interaction")
	UAOContainerInteractionSessionModel* GetCurrentContainerSessionModel() const;

	template<typename SessionModelClass>
	SessionModelClass* GetCurrentSessionModelAs() const
	{
		return Cast<SessionModelClass>(CurrentSessionModel);
	}

protected:
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedSessionState)
	FAOReplicatedInteractionSessionState ReplicatedSessionState;

	UPROPERTY(Transient)
	TObjectPtr<UAOInteractionSessionModel> CurrentSessionModel = nullptr;

	FOnCurrentSessionChanged OnCurrentSessionChanged;

protected:
	UFUNCTION()
	void OnRep_ReplicatedSessionState();

private:
	void ResetCurrentSessionModel();
	void UpdateReplicatedStateFromCurrentSession();
	void ForceSessionReplicationUpdate() const;
	void RebuildClientSessionFromReplicatedState();
	void FlushPendingCurrentInteractableMutationsIfReady();
	void ClearPendingCurrentInteractableMutations();
	void SetCurrentInteractableOwner(AActor* NewOwnerActor);

	UFUNCTION(Server, Reliable)
	void Server_RequestAcquireCurrentInteractableOwner();

	UFUNCTION(Server, Reliable)
	void Server_RequestReleaseCurrentInteractableOwner();

	UFUNCTION(Server, Reliable)
	void Server_RequestCloseCurrentSession();

	UFUNCTION(Server, Reliable)
	void Server_ExecuteCurrentContainerMutationRequest(const FAOContainerSessionMutationRequest& MutationRequest);

	TArray<FAOInteractableMutationRequest> PendingCurrentInteractableMutations;
};
