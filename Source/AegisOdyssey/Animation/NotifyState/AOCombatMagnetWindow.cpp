#include "AOCombatMagnetWindow.h"

#include "AegisOdyssey/Character/AOCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCombatMagnetWindow)

void UAOCombatMagnetWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp == nullptr)
	{
		return;
	}

	AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(MeshComp->GetOwner());
	if (OwnerCharacter == nullptr)
	{
		return;
	}

	if (UAOCharacterCombatManagerComponent* CombatManager = OwnerCharacter->FindComponentByClass<UAOCharacterCombatManagerComponent>())
	{
		CombatManager->BeginCombatMagnetWindow(WindowConfig);
	}
}

void UAOCombatMagnetWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp == nullptr)
	{
		return;
	}

	AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(MeshComp->GetOwner());
	if (OwnerCharacter == nullptr)
	{
		return;
	}

	if (UAOCharacterCombatManagerComponent* CombatManager = OwnerCharacter->FindComponentByClass<UAOCharacterCombatManagerComponent>())
	{
		CombatManager->EndCombatMagnetWindow(WindowConfig.WarpTargetName);
	}
}
