// Fill out your copyright notice in the Description page of Project Settings.


#include "AOCharacter.h"
#include "AegisOdyssey/Equipment/AOWeaponManagerComponent.h"
#include "AOCharacterMovementComponent.h"
#include "AegisOdyssey/AbilitySystem/Attributes/AOHealthAttributeSet.h"
#include "AegisOdyssey/Camera/AOCameraComponent.h"
#include "AegisOdyssey/Equipment/AOQuickBarComponent.h"
#include "AegisOdyssey/StateTree/AOStateTreeComponentBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/SpringArmComponent.h"

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
	CapsuleComp->InitCapsuleSize(50.f,90.f);
	CapsuleComp->SetCollisionProfileName(FName("AOPlayerCapsule_Name"));

	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeRotation(FRotator(0.f,-90.f,0.f));
	MeshComp->SetCollisionProfileName(FName("AOPlayerCollision_Name"));
	

	/*CreateDefaultPawnExtComp*/
	AOExtPawnComp = CreateDefaultSubobject<UAOExtPawnComponent>(TEXT("AOExtPawnComponent"));

	
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
			TreeComponent->RestartLogic(); //重置所有状态树
		}
	}
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
	
	AOExtPawnComp->HandleControllerChange();  //刷新ExtComp中的ASC信息
	HandleStateTreeChange();

}

void AAOCharacter::Reset()
{
	Super::Reset();
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

//角色初始化组件之后
void AAOCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//发送添加角色的技能事件
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this,NAME_AOAbilityReady);
}

void AAOCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	AOExtPawnComp->CheckDefaultInitialization();
}

