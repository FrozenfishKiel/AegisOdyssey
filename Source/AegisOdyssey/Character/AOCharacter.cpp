// Fill out your copyright notice in the Description page of Project Settings.


#include "AOCharacter.h"

#include "AOCharacterMovementComponent.h"
#include "AegisOdyssey/Camera/AOCameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCharacter)

AAOCharacter::AAOCharacter(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

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

	
	GetCharacterMovement()->GravityScale = 1.0f;
	GetCharacterMovement()->MaxAcceleration = 2400.0f;
	GetCharacterMovement()->BrakingFrictionFactor = 1.0f;
	GetCharacterMovement()->BrakingFriction = 6.0f;
	GetCharacterMovement()->GroundFriction = 8.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1400.0f;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
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

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
}

UAbilitySystemComponent* AAOCharacter::GetAbilitySystemComponent() const
{
	return AOSourceASC ? AOSourceASC : nullptr;
}


void AAOCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (!AOExtPawnComp) return;
	AOExtPawnComp->HandleControllerChange();
}

void AAOCharacter::UnPossessed()
{
	Super::UnPossessed();
	
	AOExtPawnComp->HandleControllerChange();  //刷新ExtComp中的ASC信息
}

void AAOCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
	if (!AOExtPawnComp) return;
	AOExtPawnComp->HandleControllerChange();
}

void AAOCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (!AOExtPawnComp) return;
	AOExtPawnComp->HandlePlayerStateReplicated();
}

void AAOCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	AOExtPawnComp->CheckDefaultInitialization();
}
