// Fill out your copyright notice in the Description page of Project Settings.


#include "AOCharacter.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"

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
}
