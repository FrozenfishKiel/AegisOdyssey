#include "AOAttackTrailWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Combat/Effects/AOAttackEffectProfile.h"
#include "AegisOdyssey/Equipment/AOWeaponManagerComponent.h"
#include "AegisOdyssey/Equipment/Weapons/AOWeaponInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOAttackTrailWindow)

namespace AOAttackTrailWindow_Private
{
	static bool BuildTrailCueContext(
		USkeletalMeshComponent* MeshComp,
		AAOCharacter*& OutCharacter,
		UAbilitySystemComponent*& OutASC,
		FGameplayCueParameters& OutCueParameters)
	{
		AActor* OwnerActor = MeshComp != nullptr ? MeshComp->GetOwner() : nullptr;
		if (!OwnerActor || !OwnerActor->HasAuthority())
		{
			return false;
		}

		AAOCharacter* AOCharacter = Cast<AAOCharacter>(OwnerActor);
		if (AOCharacter == nullptr)
		{
			return false;
		}

		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AOCharacter);
		if (ASC == nullptr)
		{
			return false;
		}

		const UAOWeaponManagerComponent* WeaponManagerComponent = AOCharacter->FindComponentByClass<UAOWeaponManagerComponent>();
		UObject* SourceObject = WeaponManagerComponent != nullptr
			? Cast<UAOWeaponInstance>(WeaponManagerComponent->GetCurrentWeaponInstance())
			: nullptr;

		OutCueParameters = FGameplayCueParameters();
		OutCueParameters.Instigator = AOCharacter;
		OutCueParameters.EffectCauser = AOCharacter;
		OutCueParameters.SourceObject = SourceObject != nullptr ? SourceObject : AOCharacter;
		OutCueParameters.Location = AOCharacter->GetActorLocation();
		OutCueParameters.Normal = AOCharacter->GetActorForwardVector();

		OutCharacter = AOCharacter;
		OutASC = ASC;
		return true;
	}
}

void UAOAttackTrailWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AAOCharacter* AOCharacter = nullptr;
	UAbilitySystemComponent* ASC = nullptr;
	FGameplayCueParameters CueParameters;
	if (!AOAttackTrailWindow_Private::BuildTrailCueContext(MeshComp, AOCharacter, ASC, CueParameters))
	{
		return;
	}

	FAOAttackEffectProfileRuntime::DispatchTrigger(
		FAOAttackEffectProfileRuntime::ResolveProfileFromActor(AOCharacter),
		EAOAttackEffectTrigger::CombatWindowBegin,
		ASC,
		CueParameters);
}

void UAOAttackTrailWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AAOCharacter* AOCharacter = nullptr;
	UAbilitySystemComponent* ASC = nullptr;
	FGameplayCueParameters CueParameters;
	if (!AOAttackTrailWindow_Private::BuildTrailCueContext(MeshComp, AOCharacter, ASC, CueParameters))
	{
		return;
	}

	FAOAttackEffectProfileRuntime::DispatchTrigger(
		FAOAttackEffectProfileRuntime::ResolveProfileFromActor(AOCharacter),
		EAOAttackEffectTrigger::CombatWindowEnd,
		ASC,
		CueParameters);
}
