// Fill out your copyright notice in the Description page of Project Settings.

#include "AOInteractionSessionComponent.h"

#include "AegisOdyssey/Inventory/AOInventoryComponent.h"
#include "AegisOdyssey/Interaction/Session/AOInteractionSessionModel.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInteractionSessionComponent)

UAOInteractionSessionComponent::UAOInteractionSessionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
}

void UAOInteractionSessionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAOInteractionSessionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FlushPendingCurrentInteractableMutationsIfReady();
}

void UAOInteractionSessionComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedSessionState, COND_OwnerOnly);
}

void UAOInteractionSessionComponent::StartSession(UAOInteractionSessionModel* NewSessionModel)
{
	if (CurrentSessionModel == NewSessionModel)
	{
		return;
	}

	ResetCurrentSessionModel();
	CurrentSessionModel = NewSessionModel;

	if (CurrentSessionModel)
	{
		CurrentSessionModel->ActivateSession(this);
	}

	// 会话一建立就沿用现有容器交互的成熟权限链，
	// 先把当前可交互对象的 owner 收到会话拥有者名下，避免 target-side 变更再临时掉回旧 RPC。
	if (GetOwner() && GetOwner()->HasAuthority() && CurrentSessionModel != nullptr)
	{
		RequestAcquireCurrentInteractableOwner();
	}

	UpdateReplicatedStateFromCurrentSession();
	ForceSessionReplicationUpdate();
	OnCurrentSessionChanged.Broadcast(CurrentSessionModel);
}

void UAOInteractionSessionComponent::CloseCurrentSession()
{
	ResetCurrentSessionModel();
	UpdateReplicatedStateFromCurrentSession();
	ForceSessionReplicationUpdate();
	OnCurrentSessionChanged.Broadcast(CurrentSessionModel);
}

void UAOInteractionSessionComponent::RequestCloseCurrentSession()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		CloseCurrentSession();
		return;
	}

	Server_RequestCloseCurrentSession();
}

void UAOInteractionSessionComponent::RequestAcquireCurrentInteractableOwner()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		SetCurrentInteractableOwner(Cast<AActor>(GetOwner()));
		return;
	}

	Server_RequestAcquireCurrentInteractableOwner();
}

void UAOInteractionSessionComponent::RequestReleaseCurrentInteractableOwner()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (AActor* Interactable = GetCurrentInteractableActor())
		{
			if (Interactable->GetOwner() == GetOwner())
			{
				SetCurrentInteractableOwner(nullptr);
			}
		}

		return;
	}

	Server_RequestReleaseCurrentInteractableOwner();
}

bool UAOInteractionSessionComponent::HasCurrentInteractableMutationAuthority() const
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		return true;
	}

	const AActor* Interactable = GetCurrentInteractableActor();
	const AActor* SessionOwnerActor = Cast<AActor>(GetOwner());
	if (Interactable == nullptr || SessionOwnerActor == nullptr)
	{
		return false;
	}

	return Interactable->GetOwner() == SessionOwnerActor;
}

bool UAOInteractionSessionComponent::SubmitCurrentInteractableMutation(FAOInteractableMutationRequest&& MutationRequest)
{
	if (!MutationRequest.IsValid())
	{
		return false;
	}

	auto TryExecuteMutation = [&MutationRequest]() -> bool
	{
		if (!MutationRequest.CanExecute())
		{
			return false;
		}

		MutationRequest.Execute();
		return true;
	};

	if (!HasActiveSession() || !GetCurrentInteractableActor())
	{
		return TryExecuteMutation();
	}

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		RequestAcquireCurrentInteractableOwner();
		return TryExecuteMutation();
	}

	if (HasCurrentInteractableMutationAuthority())
	{
		return TryExecuteMutation();
	}

	PendingCurrentInteractableMutations.Add(MoveTemp(MutationRequest));
	RequestAcquireCurrentInteractableOwner();
	return false;
}

bool UAOInteractionSessionComponent::ExecuteOrQueueCurrentInteractableMutation(TFunction<void()>&& MutationAction)
{
	return SubmitCurrentInteractableMutation(
		FAOInteractableMutationRequest(NAME_None, MoveTemp(MutationAction)));
}

bool UAOInteractionSessionComponent::ExecuteCurrentContainerMutationRequest(
	const FAOContainerSessionMutationRequest& MutationRequest)
{
	UAOContainerInteractionSessionModel* ContainerSessionModel = GetCurrentContainerSessionModel();
	if (ContainerSessionModel == nullptr)
	{
		return false;
	}

	if (!HasActiveSession() || !GetCurrentInteractableActor())
	{
		return ContainerSessionModel->ExecuteMutationRequestOnAuthority(MutationRequest);
	}

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		RequestAcquireCurrentInteractableOwner();
		return ContainerSessionModel->ExecuteMutationRequestOnAuthority(MutationRequest);
	}

	if (HasCurrentInteractableMutationAuthority())
	{
		Server_ExecuteCurrentContainerMutationRequest(MutationRequest);
		return true;
	}

	FAOContainerSessionMutationRequest DeferredRequest = MutationRequest;
	return SubmitCurrentInteractableMutation(
		FAOInteractableMutationRequest(
			TEXT("ExecuteCurrentContainerMutationRequest"),
			[this, DeferredRequest]()
			{
				const UAOContainerInteractionSessionModel* ResolvedSessionModel = GetCurrentContainerSessionModel();
				return ResolvedSessionModel != nullptr && ResolvedSessionModel->CanExecuteMutationRequest(DeferredRequest);
			},
			[this, DeferredRequest]()
			{
				Server_ExecuteCurrentContainerMutationRequest(DeferredRequest);
			}));
}

void UAOInteractionSessionComponent::SyncCurrentSessionToReplication()
{
	UpdateReplicatedStateFromCurrentSession();
	ForceSessionReplicationUpdate();
}

bool UAOInteractionSessionComponent::HasActiveSession() const
{
	return CurrentSessionModel != nullptr;
}

AActor* UAOInteractionSessionComponent::GetCurrentInteractableActor() const
{
	return CurrentSessionModel ? CurrentSessionModel->GetInteractableActor() : nullptr;
}

UAOContainerInteractionSessionModel* UAOInteractionSessionComponent::GetCurrentContainerSessionModel() const
{
	return GetCurrentSessionModelAs<UAOContainerInteractionSessionModel>();
}

void UAOInteractionSessionComponent::OnRep_ReplicatedSessionState()
{
	RebuildClientSessionFromReplicatedState();
}

void UAOInteractionSessionComponent::ResetCurrentSessionModel()
{
	if (!CurrentSessionModel)
	{
		return;
	}

	ClearPendingCurrentInteractableMutations();

	// 会话关闭或切换时，把当前交互对象的 owner 归还，
	// 避免旧会话残留 owner 继续影响后续交互权限判断。
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		RequestReleaseCurrentInteractableOwner();
	}

	CurrentSessionModel->DeactivateSession();
	CurrentSessionModel = nullptr;
}

void UAOInteractionSessionComponent::UpdateReplicatedStateFromCurrentSession()
{
	ReplicatedSessionState = FAOReplicatedInteractionSessionState();

	if (UAOContainerInteractionSessionModel* ContainerSession = Cast<UAOContainerInteractionSessionModel>(CurrentSessionModel))
	{
		ReplicatedSessionState.bHasActiveSession = true;
		ReplicatedSessionState.InteractableActor = ContainerSession->GetInteractableActor();
		ReplicatedSessionState.SessionWidgetClass = CurrentSessionModel->GetSessionWidgetClass();
		ReplicatedSessionState.ContainerInventory = ContainerSession->GetCurrentContainerInventory();
		ReplicatedSessionState.ContainerSlots = ContainerSession->GetObservedContainerSlots();
	}
}

void UAOInteractionSessionComponent::ForceSessionReplicationUpdate() const
{
	if (const AActor* OwnerActor = Cast<AActor>(GetOwner()))
	{
		const_cast<AActor*>(OwnerActor)->ForceNetUpdate();
	}
}

void UAOInteractionSessionComponent::RebuildClientSessionFromReplicatedState()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		return;
	}

	if (!ReplicatedSessionState.bHasActiveSession)
	{
		if (CurrentSessionModel)
		{
			ResetCurrentSessionModel();
			OnCurrentSessionChanged.Broadcast(CurrentSessionModel);
		}
		return;
	}

	UAOContainerInteractionSessionModel* ContainerSession = GetCurrentSessionModelAs<UAOContainerInteractionSessionModel>();
	const bool bCreatedNewSession = (ContainerSession == nullptr);
	AActor* PreviousInteractableActor = ContainerSession != nullptr ? ContainerSession->GetInteractableActor() : nullptr;
	if (!ContainerSession)
	{
		ContainerSession = NewObject<UAOContainerInteractionSessionModel>(this);
		CurrentSessionModel = ContainerSession;
	}

	ContainerSession->SetSessionWidgetClass(ReplicatedSessionState.SessionWidgetClass);
	ContainerSession->InitializeContainerSession(
		ReplicatedSessionState.InteractableActor,
		ReplicatedSessionState.ContainerInventory);

	if (bCreatedNewSession)
	{
		ContainerSession->ActivateSession(this);
	}

	ContainerSession->ApplyObservedSlotsSnapshot(ReplicatedSessionState.ContainerSlots);
	OnCurrentSessionChanged.Broadcast(CurrentSessionModel);

	// 客户端收到正式会话后，立刻补发一次权限申请，
	// 让后续 target-side 交互站回现有“先拿权限、再走旧执行语义”的成熟链上。
	if (bCreatedNewSession || PreviousInteractableActor != ReplicatedSessionState.InteractableActor)
	{
		RequestAcquireCurrentInteractableOwner();
	}

	FlushPendingCurrentInteractableMutationsIfReady();
}

void UAOInteractionSessionComponent::FlushPendingCurrentInteractableMutationsIfReady()
{
	if (PendingCurrentInteractableMutations.IsEmpty())
	{
		return;
	}

	if (!HasActiveSession() || !GetCurrentInteractableActor())
	{
		ClearPendingCurrentInteractableMutations();
		return;
	}

	if (!HasCurrentInteractableMutationAuthority())
	{
		return;
	}

	TArray<FAOInteractableMutationRequest> ReadyMutations = MoveTemp(PendingCurrentInteractableMutations);
	ClearPendingCurrentInteractableMutations();

	for (FAOInteractableMutationRequest& MutationRequest : ReadyMutations)
	{
		if (!MutationRequest.CanExecute())
		{
			continue;
		}

		MutationRequest.Execute();
	}
}

void UAOInteractionSessionComponent::ClearPendingCurrentInteractableMutations()
{
	PendingCurrentInteractableMutations.Reset();
}

void UAOInteractionSessionComponent::SetCurrentInteractableOwner(AActor* NewOwnerActor)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (AActor* Interactable = GetCurrentInteractableActor())
	{
		if (Interactable->GetOwner() != NewOwnerActor)
		{
			Interactable->SetOwner(NewOwnerActor);
			Interactable->ForceNetUpdate();
		}
	}

	FlushPendingCurrentInteractableMutationsIfReady();
}

void UAOInteractionSessionComponent::Server_RequestAcquireCurrentInteractableOwner_Implementation()
{
	SetCurrentInteractableOwner(Cast<AActor>(GetOwner()));
}

void UAOInteractionSessionComponent::Server_RequestReleaseCurrentInteractableOwner_Implementation()
{
	if (AActor* Interactable = GetCurrentInteractableActor())
	{
		if (Interactable->GetOwner() == GetOwner())
		{
			SetCurrentInteractableOwner(nullptr);
		}
	}
}

void UAOInteractionSessionComponent::Server_RequestCloseCurrentSession_Implementation()
{
	CloseCurrentSession();
}

void UAOInteractionSessionComponent::Server_ExecuteCurrentContainerMutationRequest_Implementation(
	const FAOContainerSessionMutationRequest& MutationRequest)
{
	if (UAOContainerInteractionSessionModel* ContainerSessionModel = GetCurrentContainerSessionModel())
	{
		ContainerSessionModel->ExecuteMutationRequestOnAuthority(MutationRequest);
	}
}
