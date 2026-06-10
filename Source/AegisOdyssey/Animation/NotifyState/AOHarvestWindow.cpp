// Fill out your copyright notice in the Description page of Project Settings.

#include "AOHarvestWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AegisOdyssey/AOStateTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOHarvestWindow)

UAOHarvestWindow::UAOHarvestWindow()
{
}

void UAOHarvestWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp == nullptr)
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (OwnerActor == nullptr)
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	AbilitySystemComponent->AddLooseGameplayTag(AOStateTags::State_Harvest_HitWindow);
}

void UAOHarvestWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp == nullptr)
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (OwnerActor == nullptr)
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	AbilitySystemComponent->RemoveLooseGameplayTag(AOStateTags::State_Harvest_HitWindow);
}
