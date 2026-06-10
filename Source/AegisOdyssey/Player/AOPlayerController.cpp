// Fill out your copyright notice in the Description page of Project Settings.

#include "AOPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "NativeGameplayTags.h"
#include "AegisOdyssey/AOCombatMessageSubsystem.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Crafting/Components/AOCraftingComponent.h"
#include "AegisOdyssey/Inventory/AOInventoryStatics.h"
#include "AegisOdyssey/Interaction/AOInteractionSessionComponent.h"
#include "AegisOdyssey/Interaction/InteractionStatics.h"
#include "AegisOdyssey/Interaction/Session/AOContainerInteractionSessionModel.h"
#include "AegisOdyssey/Items/AOItemCatalogTypes.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AegisOdyssey/System/AOGameData.h"
#include "AegisOdyssey/UI/WorldHealthBar/AOLocalTargetHealthBarObserverComponent.h"
#include "CommonActivatableWidget.h"
#include "EngineUtils.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOPlayerController)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Ability_Interaction_Activate_ServerForward, "Ability.Interaction.Activate");

namespace
{
static AActor* ResolveDebugTargetActor(UWorld* World, AActor* DefaultActor, const FString& TargetActorNameOrPath, FString& OutResultMessage, const TCHAR* FailureContext)
{
	if (World == nullptr)
	{
		OutResultMessage = FString::Printf(TEXT("%s failed: world is unavailable."), FailureContext);
		return nullptr;
	}

	const FString TargetToken = TargetActorNameOrPath.TrimStartAndEnd();
	if (TargetToken.IsEmpty())
	{
		if (DefaultActor == nullptr)
		{
			OutResultMessage = FString::Printf(TEXT("%s failed: controlled pawn was not found."), FailureContext);
		}
		return DefaultActor;
	}

	if (AActor* ResolvedByPath = Cast<AActor>(StaticFindObject(AActor::StaticClass(), nullptr, *TargetToken)))
	{
		if (ResolvedByPath->GetWorld() == World)
		{
			return ResolvedByPath;
		}
	}

	TArray<AActor*> MatchedActors;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->GetName() == TargetToken)
		{
			MatchedActors.Add(*It);
		}
	}

	if (MatchedActors.Num() > 1)
	{
		OutResultMessage = FString::Printf(TEXT("%s failed: multiple actors matched target name '%s'."), FailureContext, *TargetToken);
		return nullptr;
	}

	if (MatchedActors.Num() == 1)
	{
		return MatchedActors[0];
	}

	OutResultMessage = FString::Printf(TEXT("%s failed: target actor '%s' was not found."), FailureContext, *TargetToken);
	return nullptr;
}

static bool TryStartDebugCharacterInventorySession(
	AAOPlayerController* RequestingController,
	AAOCharacter* TargetCharacter,
	TSubclassOf<UCommonActivatableWidget> SessionWidgetClass)
{
	if (RequestingController == nullptr || TargetCharacter == nullptr || !SessionWidgetClass)
	{
		return false;
	}

	UAOInventoryComponent* InventoryComponent = TargetCharacter->GetInventoryComponent();
	if (InventoryComponent == nullptr)
	{
		return false;
	}

	if (UAOInteractionSessionComponent* SessionComponent = RequestingController->GetInteractionSessionComponent())
	{
		UAOContainerInteractionSessionModel* SessionModel = NewObject<UAOContainerInteractionSessionModel>(SessionComponent);
		SessionModel->InitializeContainerSession(TargetCharacter, InventoryComponent);
		SessionModel->SetSessionWidgetClass(SessionWidgetClass);
		SessionComponent->StartSession(SessionModel);
		return true;
	}

	return false;
}
}

AAOPlayerController::AAOPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 交互会话属于玩家私有状态，直接挂在 PlayerController 上最符合网络归属。
	InteractionSessionComponent = CreateDefaultSubobject<UAOInteractionSessionComponent>(TEXT("InteractionSessionComponent"));

	// 目标血条观察也属于本地玩家私有视角逻辑。
	// 目标组件管数据真相，这个组件只管“我现在要不要看它”。
	LocalTargetHealthBarObserverComponent = CreateDefaultSubobject<UAOLocalTargetHealthBarObserverComponent>(
		TEXT("LocalTargetHealthBarObserverComponent"));
}

void AAOPlayerController::CraftRecipe(FName RecipeRowName)
{
	if (!RecipeRowName.IsValid())
	{
		UE_LOG(LogAegisOdyssey, Warning, TEXT("CraftRecipe failed: RecipeRowName is invalid."));
		return;
	}

	const AAOCharacter* ControlledCharacter = Cast<AAOCharacter>(GetPawn());
	if (ControlledCharacter == nullptr)
	{
		UE_LOG(LogAegisOdyssey, Warning, TEXT("CraftRecipe failed: controlled pawn is not AAOCharacter."));
		return;
	}

	UAOCraftingComponent* CraftingComponent = ControlledCharacter->FindComponentByClass<UAOCraftingComponent>();
	if (CraftingComponent == nullptr)
	{
		UE_LOG(LogAegisOdyssey, Warning, TEXT("CraftRecipe failed: crafting component was not found on %s."), *GetNameSafe(ControlledCharacter));
		return;
	}

	const bool bAccepted = CraftingComponent->RequestEnqueueRecipe(RecipeRowName);
	UE_LOG(
		LogAegisOdyssey,
		Log,
		TEXT("CraftRecipe request for '%s' on %s returned %s."),
		*RecipeRowName.ToString(),
		*GetNameSafe(ControlledCharacter),
		bAccepted ? TEXT("true") : TEXT("false"));
}

void AAOPlayerController::GiveItem(int32 ItemId, int32 Count, FString TargetActorNameOrPath)
{
	if (ItemId == INDEX_NONE)
	{
		UE_LOG(LogAegisOdyssey, Warning, TEXT("GiveItem failed locally: ItemId is invalid."));
		return;
	}

	if (Count <= 0)
	{
		UE_LOG(LogAegisOdyssey, Warning, TEXT("GiveItem failed locally: Count must be greater than 0."));
		return;
	}

	UE_LOG(
		LogAegisOdyssey,
		Log,
		TEXT("GiveItem request sent. Requester=%s ItemId=%d Count=%d Target='%s'"),
		*GetNameSafe(this),
		ItemId,
		Count,
		TargetActorNameOrPath.IsEmpty() ? TEXT("<ControlledPawn>") : *TargetActorNameOrPath);

	Server_GiveItemRequest(ItemId, Count, TargetActorNameOrPath);
}

void AAOPlayerController::DebugOpenCharacterInventory(FString TargetActorNameOrPath)
{
	UE_LOG(
		LogAegisOdyssey,
		Log,
		TEXT("DebugOpenCharacterInventory request sent. Requester=%s Target='%s'"),
		*GetNameSafe(this),
		TargetActorNameOrPath.IsEmpty() ? TEXT("<ControlledPawn>") : *TargetActorNameOrPath);

	Server_DebugOpenCharacterInventoryRequest(TargetActorNameOrPath);
}

void AAOPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PostProcessInput(DeltaTime, bGamePaused);

	if (UAOAbilitySystem* AOASC = Cast<UAOAbilitySystem>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn())))
	{
		AOASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}
}

void AAOPlayerController::ClientReportGiveItemResult_Implementation(bool bSucceeded, const FString& ResultMessage)
{
	if (bSucceeded)
	{
		UE_LOG(LogAegisOdyssey, Log, TEXT("%s"), *ResultMessage);
	}
	else
	{
		UE_LOG(LogAegisOdyssey, Warning, TEXT("%s"), *ResultMessage);
	}
}

void AAOPlayerController::ClientBroadcastCombatResultMessage_Implementation(const FAOCombatResultMessage& Message)
{
	if (UAOCombatMessageSubsystem* CombatMessageSubsystem = UAOCombatMessageSubsystem::Get(this))
	{
		CombatMessageSubsystem->BroadcastCombatResultLocal(Message);
	}
}

void AAOPlayerController::Server_ExecuteInteractionRequest_Implementation(AActor* TargetActor, FName TargetComponentName, int32 OptionIndex)
{
	APawn* ControlledPawn = GetPawn();
	TScriptInterface<IInteractableTarget> InteractableTarget = UInteractionStatics::ResolveInteractableTarget(TargetActor, TargetComponentName);
	if (!ControlledPawn || !InteractableTarget)
	{
		return;
	}

	// 服务端继续沿用统一交互对象链执行，不在这里关心对象类型。
	FGameplayEventData Payload;
	Payload.EventTag = TAG_Ability_Interaction_Activate_ServerForward;
	Payload.Instigator = ControlledPawn;
	Payload.Target = TargetActor;
	Payload.EventMagnitude = static_cast<float>(OptionIndex);

	UInteractionStatics::TryExecuteInteraction(InteractableTarget, TAG_Ability_Interaction_Activate_ServerForward, Payload);
}

void AAOPlayerController::Server_GiveItemRequest_Implementation(int32 ItemId, int32 Count, const FString& TargetActorNameOrPath)
{
	FString ResultMessage;
	const bool bSucceeded = TryExecuteGiveItemOnAuthority(ItemId, Count, TargetActorNameOrPath, ResultMessage);

	if (bSucceeded)
	{
		UE_LOG(
			LogAegisOdyssey,
			Log,
			TEXT("GiveItem authority result. Requester=%s ItemId=%d Count=%d Target='%s' Result=%s"),
			*GetNameSafe(this),
			ItemId,
			Count,
			TargetActorNameOrPath.IsEmpty() ? TEXT("<ControlledPawn>") : *TargetActorNameOrPath,
			*ResultMessage);
	}
	else
	{
		UE_LOG(
			LogAegisOdyssey,
			Warning,
			TEXT("GiveItem authority result. Requester=%s ItemId=%d Count=%d Target='%s' Result=%s"),
			*GetNameSafe(this),
			ItemId,
			Count,
			TargetActorNameOrPath.IsEmpty() ? TEXT("<ControlledPawn>") : *TargetActorNameOrPath,
			*ResultMessage);
	}

	ClientReportGiveItemResult(bSucceeded, ResultMessage);
}

void AAOPlayerController::Server_DebugOpenCharacterInventoryRequest_Implementation(const FString& TargetActorNameOrPath)
{
	FString ResultMessage;
	const bool bSucceeded = TryExecuteDebugOpenCharacterInventoryOnAuthority(TargetActorNameOrPath, ResultMessage);

	if (bSucceeded)
	{
		UE_LOG(
			LogAegisOdyssey,
			Log,
			TEXT("DebugOpenCharacterInventory authority result. Requester=%s Target='%s' Result=%s"),
			*GetNameSafe(this),
			TargetActorNameOrPath.IsEmpty() ? TEXT("<ControlledPawn>") : *TargetActorNameOrPath,
			*ResultMessage);
	}
	else
	{
		UE_LOG(
			LogAegisOdyssey,
			Warning,
			TEXT("DebugOpenCharacterInventory authority result. Requester=%s Target='%s' Result=%s"),
			*GetNameSafe(this),
			TargetActorNameOrPath.IsEmpty() ? TEXT("<ControlledPawn>") : *TargetActorNameOrPath,
			*ResultMessage);
	}

	ClientReportGiveItemResult(bSucceeded, ResultMessage);
}

bool AAOPlayerController::TryExecuteGiveItemOnAuthority(
	int32 ItemId,
	int32 Count,
	const FString& TargetActorNameOrPath,
	FString& OutResultMessage)
{
	if (ItemId == INDEX_NONE)
	{
		OutResultMessage = TEXT("GiveItem failed: ItemId is invalid.");
		return false;
	}

	if (Count <= 0)
	{
		OutResultMessage = TEXT("GiveItem failed: Count must be greater than 0.");
		return false;
	}

	UWorld* World = GetWorld();
	AActor* TargetActor = ResolveDebugTargetActor(World, GetPawn(), TargetActorNameOrPath, OutResultMessage, TEXT("GiveItem"));
	if (TargetActor == nullptr)
	{
		return false;
	}

	const FAOItemCatalogRow* ItemCatalogRow = UAOGameData::Get().FindItemCatalogRowById(ItemId);
	if (ItemCatalogRow == nullptr || ItemCatalogRow->ItemDefinitionClass == nullptr)
	{
		OutResultMessage = FString::Printf(
			TEXT("GiveItem failed: ItemId %d could not resolve to a valid item definition."),
			ItemId);
		return false;
	}

	FAOInventoryReceiveBatch ReceiveBatch;
	FAOInventoryDefinitionEntry& DefinitionEntry = ReceiveBatch.DefinitionEntries.AddDefaulted_GetRef();
	DefinitionEntry.Count = Count;
	DefinitionEntry.ItemDefinitionClass = ItemCatalogRow->ItemDefinitionClass;

	if (!UAOInventoryStatics::TryAddInventoryBatchToActor(TargetActor, ReceiveBatch))
	{
		OutResultMessage = FString::Printf(
			TEXT("GiveItem failed: target actor '%s' could not accept the requested inventory batch."),
			*GetNameSafe(TargetActor));
		return false;
	}

	OutResultMessage = FString::Printf(
		TEXT("GiveItem succeeded: added ItemId=%d Count=%d to %s."),
		ItemId,
		Count,
		*GetNameSafe(TargetActor));
	return true;
}

bool AAOPlayerController::TryExecuteDebugOpenCharacterInventoryOnAuthority(
	const FString& TargetActorNameOrPath,
	FString& OutResultMessage)
{
	UWorld* World = GetWorld();
	AActor* TargetActor =
		ResolveDebugTargetActor(World, GetPawn(), TargetActorNameOrPath, OutResultMessage, TEXT("DebugOpenCharacterInventory"));
	if (TargetActor == nullptr)
	{
		return false;
	}

	AAOCharacter* TargetCharacter = Cast<AAOCharacter>(TargetActor);
	if (TargetCharacter == nullptr)
	{
		OutResultMessage = FString::Printf(
			TEXT("DebugOpenCharacterInventory failed: target '%s' is not an AAOCharacter."),
			*GetNameSafe(TargetActor));
		return false;
	}

	AAOCharacter* RequestingCharacter = Cast<AAOCharacter>(GetPawn());
	if (RequestingCharacter == nullptr)
	{
		OutResultMessage = TEXT("DebugOpenCharacterInventory failed: requesting pawn is not an AAOCharacter.");
		return false;
	}

	if (RequestingCharacter != TargetCharacter && TargetCharacter->IsPlayerControlled() && !TargetCharacter->CanOpenInventoryAsContainer())
	{
		OutResultMessage = FString::Printf(
			TEXT("DebugOpenCharacterInventory failed: target '%s' is not allowed for the current requester."),
			*GetNameSafe(TargetCharacter));
		return false;
	}

	const FInteractionOption* DefaultOption = TargetCharacter->GetDefaultInventoryInteractionOption();
	if (DefaultOption == nullptr)
	{
		OutResultMessage = FString::Printf(
			TEXT("DebugOpenCharacterInventory failed: target '%s' has no valid inventory interaction widget."),
			*GetNameSafe(TargetCharacter));
		return false;
	}

	TSubclassOf<UCommonActivatableWidget> SessionWidgetClass = DefaultOption->InteractionWidgetClass.LoadSynchronous();
	if (!SessionWidgetClass)
	{
		OutResultMessage = FString::Printf(
			TEXT("DebugOpenCharacterInventory failed: target '%s' has no valid inventory interaction widget."),
			*GetNameSafe(TargetCharacter));
		return false;
	}

	if (!TryStartDebugCharacterInventorySession(this, TargetCharacter, SessionWidgetClass))
	{
		OutResultMessage = FString::Printf(
			TEXT("DebugOpenCharacterInventory failed: target '%s' could not start an inventory interaction session."),
			*GetNameSafe(TargetCharacter));
		return false;
	}

	OutResultMessage = FString::Printf(
		TEXT("DebugOpenCharacterInventory succeeded: opened inventory for %s."),
		*GetNameSafe(TargetCharacter));
	return true;
}
