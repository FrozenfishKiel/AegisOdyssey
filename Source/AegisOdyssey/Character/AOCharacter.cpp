// Fill out your copyright notice in the Description page of Project Settings.


#include "AOCharacter.h"

#include "AOCharacterCombatManagerComponent.h"
#include "Enemies/AOEnemy.h"
#include "Enemies/AI/Decision/AOAIDecisionComponent.h"
#include "AOPersistentStateTagComponent.h"
#include "AegisOdyssey/Interaction/AOInteractionSessionComponent.h"
#include "AegisOdyssey/Interaction/InteractionStatics.h"
#include "AegisOdyssey/Interaction/Session/AOContainerInteractionSessionModel.h"
#include "AegisOdyssey/Equipment/AOWeaponManagerComponent.h"
#include "AOCharacterMovementComponent.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOHealthAttributeSet.h"
#include "AegisOdyssey/Camera/AOCameraComponent.h"
#include "AegisOdyssey/Crafting/Components/AOCraftingComponent.h"
#include "AegisOdyssey/Equipment/AOQuickBarComponent.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentManagerComponent.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentSlotInventoryComponent.h"
#include "AegisOdyssey/SkillSystem/Components/AOSkillComponent.h"
#include "AegisOdyssey/SkillSystem/Components/AOSkillSlotInventoryComponent.h"
#include "AegisOdyssey/StateTree/AOStateTreeComponentBase.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AegisOdyssey/Player/AOPlayerController.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/SpringArmComponent.h"
#include "MotionWarpingComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCharacter)
const FName AAOCharacter::NAME_AOAbilityReady("AOAbilitiesReady");

AAOCharacter::AAOCharacter(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicateUsingRegisteredSubObjectList = true;

	SetNetCullDistanceSquared(900000000000.0f);
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(50.f, 90.f);
	CapsuleComp->SetCollisionProfileName(FName("AOPlayerCapsule_Name"));

	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	MeshComp->SetCollisionProfileName(FName("AOPlayerCollision_Name"));

	InteractionBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBounds"));
	InteractionBounds->SetupAttachment(GetRootComponent());
	InteractionBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBounds->SetCollisionObjectType(ECC_GameTraceChannel1);
	InteractionBounds->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBounds->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	InteractionBounds->SetBoxExtent(FVector(55.0f, 55.0f, 96.0f));

	/*CreateDefaultPawnExtComp*/
	AOExtPawnComp = CreateDefaultSubobject<UAOExtPawnComponent>(TEXT("AOExtPawnComponent"));
	CharacterCombatManagerComponent = CreateDefaultSubobject<UAOCharacterCombatManagerComponent>(TEXT("AOCharacterCombatManagerComponent"));
	PersistentStateTagComponent = CreateDefaultSubobject<UAOPersistentStateTagComponent>(TEXT("PersistentStateTagComponent"));
	AIDecisionComponent = CreateDefaultSubobject<UAOAIDecisionComponent>(TEXT("AIDecisionComponent"));
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->bUsePawnControlRotation = true;

	/*CreateDefaultPawnExtComp*/
	AOCameraComponent = CreateDefaultSubobject<UAOCameraComponent>(TEXT("AOCameraComponent"));
	AOCameraComponent->SetupAttachment(SpringArmComponent);
	AOCameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));
	AOCameraComponent->bUsePawnControlRotation = true;

	CharacterQuickBar = CreateDefaultSubobject<UAOQuickBarComponent>(TEXT("CharacterQuickBarComponent"));
	CharacterBackPackComponent = CreateDefaultSubobject<UAOBackPackComponent>(TEXT("CharacterBackPackComponent"));
	// 技能组件和 QuickBar / BackPack 一样，作为角色身上的常驻运行时系统存在。
	// 第一阶段先把组件挂载点固定下来，避免后面再去做“系统住在哪里”的返工。
	CharacterSkillComponent = CreateDefaultSubobject<UAOSkillComponent>(TEXT("CharacterSkillComponent"));
	// 技能槽库存适配层和 SkillComponent 配套存在。
	// SkillComponent 管运行时真相，SkillSlotInventory 负责复用库存拖拽/交换语义。
	CharacterSkillSlotInventoryComponent = CreateDefaultSubobject<UAOSkillSlotInventoryComponent>(TEXT("CharacterSkillSlotInventoryComponent"));
	CharacterCraftingComponent = CreateDefaultSubobject<UAOCraftingComponent>(TEXT("CharacterCraftingComponent"));
	// 正式装备栏独立于 WeaponManager。
	// WeaponManager 继续只处理当前武器激活态，正式装备栏单独维护长期穿戴真相。
	CharacterFormalEquipmentManagerComponent = CreateDefaultSubobject<UAOFormalEquipmentManagerComponent>(TEXT("CharacterFormalEquipmentManagerComponent"));
	CharacterFormalEquipmentSlotInventoryComponent = CreateDefaultSubobject<UAOFormalEquipmentSlotInventoryComponent>(TEXT("CharacterFormalEquipmentSlotInventoryComponent"));

	WeaponInventoryManager = CreateDefaultSubobject<UAOWeaponManagerComponent>(TEXT("InventoryManagerComponent"));
	WeaponInventoryManager->SetIsReplicated(true);

	GetCharacterMovement()->GravityScale = 1.0f;
	GetCharacterMovement()->MaxAcceleration = 2400.0f;
	GetCharacterMovement()->BrakingFrictionFactor = 1.0f;
	GetCharacterMovement()->BrakingFriction = 6.0f;
	GetCharacterMovement()->GroundFriction = 8.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1400.0f;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	GetCharacterMovement()->bAllowPhysicsRotationDuringAnimRootMotion = false;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
	GetCharacterMovement()->SetCrouchedHalfHeight(65.0f);

	/*CreateDefaultAbilitySystemComponent*/
	AOSourceASC = CreateDefaultSubobject<UAOAbilitySystem>(TEXT("AOAbilitySystem"));
	AOSourceASC->SetIsReplicated(true);
	AOSourceASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	SetNetUpdateFrequency(100.f);

	HealthAttributes = CreateDefaultSubobject<UAOHealthAttributeSet>(TEXT("HealthAttributes"));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	BaseEyeHeight = 80.0f;
	CrouchedEyeHeight = 50.0f;
}

UAbilitySystemComponent* AAOCharacter::GetAbilitySystemComponent() const
{
	return AOSourceASC ? AOSourceASC : nullptr;
}

USceneComponent* AAOCharacter::GetEquipmentAttachTargetByTag(FName Tag) const
{
	USceneComponent* FoundComponent = FindComponentByTag<USceneComponent>(Tag);
	if (FoundComponent)
	{
		return FoundComponent;
	}

	return GetMesh();
}

EEquipState AAOCharacter::FindEquipState() const
{
	if (!GetAbilitySystemComponent()) return EEquipState::None;
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

	FGameplayTagContainer AllTags = ASC->GetOwnedGameplayTags();

	for (const TPair<FGameplayTag, EEquipState>& Pair : TagToEquipState)
	{
		if (AllTags.HasTagExact(Pair.Key))
		{
			return Pair.Value;
		}
	}
	return EEquipState::None;
}

void AAOCharacter::GatherInteractionOptions(FInteractionOptionBuilder& OptionBuilder)
{
	if (!CanOpenInventoryAsContainer())
	{
		return;
	}

	for (const FInteractionOption& InteractionOption : InventoryInteractionOptions)
	{
		OptionBuilder.AddInteractionOption(InteractionOption);
	}
}

bool AAOCharacter::CanExecuteInteraction(const FGameplayTag& InteractionEventTag, const FGameplayEventData& EventData) const
{
	if (!HasAuthority() || !CanOpenInventoryAsContainer())
	{
		return false;
	}

	const APawn* InteractingPawn = Cast<APawn>(const_cast<AActor*>(Cast<AActor>(EventData.Instigator.Get())));
	if (!CanOpenInventoryForInteractor(InteractingPawn))
	{
		return false;
	}

	const int32 SelectedInteractionIndex = UInteractionStatics::GetInteractionOptionIndexFromEventData(EventData);
	return FindInventoryInteractionOptionByIndex(SelectedInteractionIndex) != nullptr;
}

bool AAOCharacter::ExecuteInteraction(const FGameplayTag& InteractionEventTag, const FGameplayEventData& EventData)
{
	if (!HasAuthority())
	{
		return false;
	}

	APawn* InteractingPawn = Cast<APawn>(const_cast<AActor*>(Cast<AActor>(EventData.Instigator.Get())));
	if (!CanOpenInventoryForInteractor(InteractingPawn))
	{
		return false;
	}

	AAOPlayerController* PlayerController = Cast<AAOPlayerController>(InteractingPawn->GetController());
	UAOInventoryComponent* InventoryComponent = GetInventoryComponent();
	if (!PlayerController || InventoryComponent == nullptr)
	{
		return false;
	}

	const int32 SelectedInteractionIndex = UInteractionStatics::GetInteractionOptionIndexFromEventData(EventData);
	const FInteractionOption* SelectedOption = FindInventoryInteractionOptionByIndex(SelectedInteractionIndex);
	if (!SelectedOption)
	{
		return false;
	}

	if (UAOInteractionSessionComponent* SessionComponent = PlayerController->GetInteractionSessionComponent())
	{
		// 这轮先把角色背包接入既有容器会话，避免提前扩成角色专用多容器 UI。
		UAOContainerInteractionSessionModel* SessionModel = NewObject<UAOContainerInteractionSessionModel>(SessionComponent);
		SessionModel->InitializeContainerSession(this, InventoryComponent);
		SessionModel->SetSessionWidgetClass(SelectedOption->InteractionWidgetClass.LoadSynchronous());
		SessionComponent->StartSession(SessionModel);
		return true;
	}

	return false;
}

UAOInventoryComponent* AAOCharacter::GetInventoryComponent()
{
	return CharacterBackPackComponent;
}

const UAOInventoryComponent* AAOCharacter::GetInventoryComponent() const
{
	return CharacterBackPackComponent;
}

bool AAOCharacter::CanOpenInventoryAsContainer() const
{
	if (GetInventoryComponent() == nullptr)
	{
		return false;
	}

	if (IsPlayerControlled() && !IsDeadForInventoryInteraction())
	{
		return false;
	}

	//if (IsA<AAOEnemy>() && !IsDeadForInventoryInteraction())
	//{
	//	return false;
	//}

	return true;
}

void AAOCharacter::DisableMovementAndCollision()
{
	if (Controller)
	{
		Controller->SetIgnoreMoveInput(true);
	}

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	UAOCharacterMovementComponent* AOMoveComp = CastChecked<UAOCharacterMovementComponent>(GetCharacterMovement());
	AOMoveComp->StopMovementImmediately();
	AOMoveComp->DisableMovement();
}

void AAOCharacter::HandleStateTreeChange()
{
	for (UActorComponent* Component : GetComponents())
	{
		if (Component->IsA(UAOStateTreeComponentBase::StaticClass()))
		{
			UAOStateTreeComponentBase* TreeComponent = CastChecked<UAOStateTreeComponentBase>(Component);
			TreeComponent->RestartLogic();
		}
	}
}

bool AAOCharacter::CanOpenInventoryForInteractor(const APawn* InteractingPawn) const
{
	if (InteractingPawn == nullptr)
	{
		return false;
	}

	if (!CanOpenInventoryAsContainer())
	{
		return false;
	}

	const AAOCharacter* InteractingCharacter = Cast<AAOCharacter>(InteractingPawn);
	if (InteractingCharacter == nullptr)
	{
		return false;
	}

	if (InteractingCharacter == this)
	{
		return true;
	}

	if (IsDeadForInventoryInteraction())
	{
		return true;
	}

	if (IsPlayerControlled())
	{
		return false;
	}

	return !IsA<AAOEnemy>();
}

bool AAOCharacter::IsDeadForInventoryInteraction() const
{
	const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
	if (AbilitySystemComponent == nullptr)
	{
		return false;
	}

	return AbilitySystemComponent->GetNumericAttribute(UAOHealthAttributeSet::GetHealthAttribute()) <= KINDA_SMALL_NUMBER;
}

const FInteractionOption* AAOCharacter::FindInventoryInteractionOptionByIndex(int32 InteractionOptionIndex) const
{
	if (!InventoryInteractionOptions.IsValidIndex(InteractionOptionIndex))
	{
		return nullptr;
	}

	return &InventoryInteractionOptions[InteractionOptionIndex];
}

const FInteractionOption* AAOCharacter::GetDefaultInventoryInteractionOption() const
{
	for (const FInteractionOption& InteractionOption : InventoryInteractionOptions)
	{
		if (InteractionOption.InteractionWidgetClass.IsNull())
		{
			continue;
		}

		if (InteractionOption.InteractionWidgetClass.LoadSynchronous() != nullptr)
		{
			return &InteractionOption;
		}
	}

	return nullptr;
}
void AAOCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (!AOExtPawnComp) return;
	AOExtPawnComp->HandleControllerChange();
	HandleStateTreeChange();
}

void AAOCharacter::UnPossessed()
{
	Super::UnPossessed();

	if (CharacterCraftingComponent)
	{
		CharacterCraftingComponent->HandleOwnerRuntimeInterrupted();
	}

	AOExtPawnComp->HandleControllerChange();
	HandleStateTreeChange();
}

void AAOCharacter::Reset()
{
	Super::Reset();

	if (CharacterCraftingComponent)
	{
		CharacterCraftingComponent->HandleOwnerRuntimeInterrupted();
	}

	DisableMovementAndCollision();
	HandleStateTreeChange();
}

void AAOCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
	if (!AOExtPawnComp) return;
	AOExtPawnComp->HandleControllerChange();
	HandleStateTreeChange();
}

void AAOCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (!AOExtPawnComp) return;
	AOExtPawnComp->HandlePlayerStateReplicated();
	HandleStateTreeChange();
}

// 角色初始化组件完成后，向框架广播能力已就绪事件。
void AAOCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_AOAbilityReady);
}

void AAOCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	AOExtPawnComp->CheckDefaultInitialization();
}

void AAOCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAOCharacter, CharacterLevel);
	DOREPLIFETIME(AAOCharacter, CharacterXP);
	DOREPLIFETIME(AAOCharacter, AvailableAttributePoints);
}

void AAOCharacter::SetCharacterLevel(int32 NewLevel)
{
	if (NewLevel < 1) NewLevel = 1;
	if (CharacterLevel == NewLevel) return;

	int32 OldLevel = CharacterLevel;
	CharacterLevel = NewLevel;

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, CharacterLevel, this);
	OnCharacterLevelChangedDelegate.Broadcast(OldLevel, CharacterLevel);
}

void AAOCharacter::AddToCharacterLevel(int32 DeltaLevel)
{
	SetCharacterLevel(CharacterLevel + DeltaLevel);
}

void AAOCharacter::SetCharacterXP(int32 NewXP)
{
	if (NewXP < 0) NewXP = 0;
	if (CharacterXP == NewXP) return;

	int32 OldXP = CharacterXP;
	CharacterXP = NewXP;

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, CharacterXP, this);
	OnCharacterXPChangedDelegate.Broadcast(OldXP, CharacterXP);
}

void AAOCharacter::AddToCharacterXP(int32 DeltaXP)
{
	SetCharacterXP(CharacterXP + DeltaXP);
	TryLevelUp();
}

bool AAOCharacter::TryLevelUp()
{
	if (!AOExtPawnComp) return false;

	const UAOPawnData* PawnData = AOExtPawnComp->GetPawnData<UAOPawnData>();
	if (!PawnData) return false;

	bool bLeveledUp = false;

	while (true)
	{
		int32 NextLevel = CharacterLevel + 1;
		int32 XPRequired = GetXPRequiredForLevel(NextLevel);

		if (CharacterXP >= XPRequired)
		{
			int32 OldLevel = CharacterLevel;
			int32 OldPoints = AvailableAttributePoints;

			CharacterLevel = NextLevel;
			int32 AttributePoints = GetAttributePointsForLevel(NextLevel);
			AvailableAttributePoints += AttributePoints;

			MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, CharacterLevel, this);
			MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, AvailableAttributePoints, this);

			OnCharacterLevelChangedDelegate.Broadcast(OldLevel, CharacterLevel);
			OnAttributePointsChangedDelegate.Broadcast(OldPoints, AvailableAttributePoints);

			bLeveledUp = true;
		}
		else
		{
			break;
		}
	}

	return bLeveledUp;
}

int32 AAOCharacter::GetXPRequiredForLevel(int32 Level) const
{
	if (!AOExtPawnComp) return 0;

	const UAOPawnData* PawnData = AOExtPawnComp->GetPawnData<UAOPawnData>();
	if (!PawnData) return 0;

	return PawnData->GetXPRequiredForLevel(Level);
}

int32 AAOCharacter::GetAttributePointsForLevel(int32 Level) const
{
	if (!AOExtPawnComp) return 0;

	const UAOPawnData* PawnData = AOExtPawnComp->GetPawnData<UAOPawnData>();
	if (!PawnData) return 0;

	return PawnData->GetAttributePointsForLevel(Level);
}

void AAOCharacter::SetAvailableAttributePoints(int32 NewPoints)
{
	if (NewPoints < 0) NewPoints = 0;
	if (AvailableAttributePoints == NewPoints) return;

	int32 OldPoints = AvailableAttributePoints;
	AvailableAttributePoints = NewPoints;

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, AvailableAttributePoints, this);
	OnAttributePointsChangedDelegate.Broadcast(OldPoints, AvailableAttributePoints);
}

void AAOCharacter::AddToAvailableAttributePoints(int32 DeltaPoints)
{
	SetAvailableAttributePoints(AvailableAttributePoints + DeltaPoints);
}

void AAOCharacter::OnRep_CharacterLevel(int32 OldLevel)
{
	OnCharacterLevelChangedDelegate.Broadcast(OldLevel, CharacterLevel);
}

void AAOCharacter::OnRep_CharacterXP(int32 OldXP)
{
	OnCharacterXPChangedDelegate.Broadcast(OldXP, CharacterXP);
}

void AAOCharacter::OnRep_AvailableAttributePoints(int32 OldPoints)
{
	OnAttributePointsChangedDelegate.Broadcast(OldPoints, AvailableAttributePoints);
}

void AAOCharacter::ApplyDamageToTarget(const FAttackedInfo& AttackedInfo)
{
	UE_LOG(
		LogAegisOdysseyCombatTrace,
		Warning,
		TEXT("[CombatTrace][Character] ApplyDamageToTarget entry. Source=%s Target=%s SourceASC=%s TargetASC=%s AttackTag=%s SkillTag=%s WeaponTag=%s"),
		*GetNameSafe(this),
		*GetNameSafe(AttackedInfo.HitResult.GetActor()),
		*GetNameSafe(AttackedInfo.SourceASC.Get()),
		*GetNameSafe(AttackedInfo.TargetASC.Get()),
		*AttackedInfo.AttackTag.ToString(),
		*AttackedInfo.SkillTag.ToString(),
		*AttackedInfo.WeaponTag.ToString());

	if (CharacterCombatManagerComponent)
	{
		CharacterCombatManagerComponent->ApplyDamageToTarget(AttackedInfo);
	}
}
