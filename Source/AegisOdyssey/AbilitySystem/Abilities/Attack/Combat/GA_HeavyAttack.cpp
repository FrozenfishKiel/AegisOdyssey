// Fill out your copyright notice in the Description page of Project Settings.

#include "GA_HeavyAttack.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AegisOdyssey/AOLogChannels.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySet.h"
#include "AegisOdyssey/AbilitySystem/Tasks/AT_WaitMovementInput.h"
#include "AegisOdyssey/AbilitySystem/Tasks/AT_WaitRotateToDirection.h"
#include "AegisOdyssey/Animation/AOAnimInstance.h"
#include "AegisOdyssey/Animation/NotifyState/AOCombatWindow.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Equipment/AOWeaponManagerComponent.h"
#include "AegisOdyssey/Equipment/Weapons/AOWeaponDefinition.h"
#include "AegisOdyssey/Equipment/Weapons/AOWeaponInstance.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "InterchangeTranslatorBase.h"
#include "Kismet/KismetSystemLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_HeavyAttack)

namespace AOHeavyAttackTracePrivate
{
	constexpr float TraceRadius = 12.0f;
	constexpr float DebugDrawSeconds = 0.03f;

	static FString BuildHitActorSummary(const TArray<FHitResult>& Hits)
	{
		TArray<FString> ActorNames;
		ActorNames.Reserve(Hits.Num());
		for (const FHitResult& Hit : Hits)
		{
			ActorNames.Add(GetNameSafe(Hit.GetActor()));
		}

		return ActorNames.Num() > 0
			? FString::Join(ActorNames, TEXT(","))
			: FString(TEXT("None"));
	}

	static void AppendTraceHits(
		UObject* WorldContextObject,
		const FVector& CurrentTraceStart,
		const FVector& CurrentTraceEnd,
		const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
		const TArray<AActor*>& IgnoreActors,
		TArray<FHitResult>& OutHits)
	{
		UKismetSystemLibrary::SphereTraceMultiForObjects(
			WorldContextObject,
			CurrentTraceStart,
			CurrentTraceEnd,
			TraceRadius,
			ObjectTypes,
			false,
			IgnoreActors,
			EDrawDebugTrace::ForOneFrame,
			OutHits,
			true,
			FLinearColor::Red,
			FLinearColor::Green,
			DebugDrawSeconds);
	}

	static void AppendContinuousTraceHits(
		UObject* WorldContextObject,
		const USceneComponent* TraceComponent,
		const FVector& CurrentTraceStart,
		const FVector& CurrentTraceEnd,
		const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
		const TArray<AActor*>& IgnoreActors,
		TMap<TObjectKey<USceneComponent>, FVector>& PreviousTraceStartByComponent,
		TMap<TObjectKey<USceneComponent>, FVector>& PreviousTraceEndByComponent,
		TArray<FHitResult>& OutHits,
		int32& OutBridgeTraceCount,
		int32& OutDirectHitCount,
		int32& OutBridgeHitCount)
	{
		OutBridgeTraceCount = 0;
		OutDirectHitCount = 0;
		OutBridgeHitCount = 0;

		TArray<FHitResult> DirectHits;
		AppendTraceHits(
			WorldContextObject,
			CurrentTraceStart,
			CurrentTraceEnd,
			ObjectTypes,
			IgnoreActors,
			DirectHits);
		OutDirectHitCount = DirectHits.Num();
		OutHits.Append(DirectHits);

		if (!TraceComponent)
		{
			return;
		}

		const TObjectKey<USceneComponent> ComponentKey(TraceComponent);
		const FVector* PreviousTraceStart = PreviousTraceStartByComponent.Find(ComponentKey);
		const FVector* PreviousTraceEnd = PreviousTraceEndByComponent.Find(ComponentKey);

		if (PreviousTraceStart && PreviousTraceEnd)
		{
			TArray<FHitResult> BridgeHits;
			if (!PreviousTraceStart->Equals(CurrentTraceStart))
			{
				AppendTraceHits(
					WorldContextObject,
					*PreviousTraceStart,
					CurrentTraceStart,
					ObjectTypes,
					IgnoreActors,
					BridgeHits);
				++OutBridgeTraceCount;
			}

			if (!PreviousTraceEnd->Equals(CurrentTraceEnd))
			{
				AppendTraceHits(
					WorldContextObject,
					*PreviousTraceEnd,
					CurrentTraceEnd,
					ObjectTypes,
					IgnoreActors,
					BridgeHits);
				++OutBridgeTraceCount;
			}

			OutBridgeHitCount = BridgeHits.Num();
			OutHits.Append(BridgeHits);
		}

		PreviousTraceStartByComponent.Add(ComponentKey, CurrentTraceStart);
		PreviousTraceEndByComponent.Add(ComponentKey, CurrentTraceEnd);
	}
}

UGA_HeavyAttack::UGA_HeavyAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InputType = EInputType::None;
	Montage = nullptr;
	PlayRate = 0.f;
	StartTime = 0.f;
	StartSection = NAME_None;
	RotationInterpSpeed = 360.0f;
}

bool UGA_HeavyAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return GetAvatarActorFromActorInfo() != nullptr;
}

void UGA_HeavyAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (TriggerEventData)
	{
		for (const TSharedPtr<FGameplayAbilityTargetData>& Data : TriggerEventData->TargetData.Data)
		{
			if (Data.IsValid() && Data->GetScriptStruct() == FHeavyAttackTargetData::StaticStruct())
			{
				FHeavyAttackTargetData* HeavyAttackData = static_cast<FHeavyAttackTargetData*>(Data.Get());
				InputTag = HeavyAttackData->InputTag;
				InputType = HeavyAttackData->InputType;
				Montage = HeavyAttackData->Montage.Get();
				PlayRate = HeavyAttackData->PlayRate;
				StartSection = HeavyAttackData->StartSection;
				StartTime = HeavyAttackData->StartTime;
				WeaponInstance = HeavyAttackData->DataWeaponInstance;
				break;
			}
		}
	}

	if (!WaitCombatHit && WeaponInstance.IsValid())
	{
		WaitCombatHit = UAT_WaitHeavyAttackCombatHit::WaitCombatHit(this, const_cast<UAOWeaponInstance*>(WeaponInstance.Get()));
		WaitCombatHit->ReadyForActivation();
	}

	if (!MovementInputTask)
	{
		MovementInputTask = UAT_WaitMovementInput::WaitMovementInput(this);
		if (MovementInputTask)
		{
			MovementInputTask->OnMovementInputDetected.AddDynamic(this, &UGA_HeavyAttack::OnMovementInputDetected);
			MovementInputTask->ReadyForActivation();
		}
	}
	else
	{
		MovementInputTask->EndTask();
		MovementInputTask = nullptr;
	}

	SetCharacterRotationToAttackDirection();
	PlayMontageAnimation();
}

void UGA_HeavyAttack::PlayMontageAnimation()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	UAnimInstance* AnimInstance = ActorInfo ? ActorInfo->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		UE_LOG(LogAegisOdysseyAbilitySystem, Error, TEXT("UGA_HeavyAttack::PlayMontageAnimation: AnimInstance is null"));
		return;
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		FName("PlayHeavyAttackMontage"),
		Montage,
		PlayRate,
		StartSection,
		true,
		1.0f,
		StartTime,
		false);

	if (MontageTask)
	{
		MontageTask->OnBlendOut.AddDynamic(this, &UGA_HeavyAttack::OnMontageBlendedOut);
		MontageTask->OnCompleted.AddDynamic(this, &UGA_HeavyAttack::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_HeavyAttack::OnMontageInterrupted);
		MontageTask->OnCancelled.AddDynamic(this, &UGA_HeavyAttack::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}
}

void UGA_HeavyAttack::OnMovementInputDetected()
{
	if (!CancelAbilityTag.IsValid())
	{
		return;
	}

	if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(CancelAbilityTag))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

void UGA_HeavyAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_HeavyAttack::OnMontageBlendedOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_HeavyAttack::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_HeavyAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_HeavyAttack::ClearCombatTags()
{
	AAOCharacter* AOCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo());
	if (!AOCharacter)
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AOCharacter);
	if (!ASC)
	{
		return;
	}

	TArray<FGameplayTag> CombatTags;
	if (Montage)
	{
		GetCombatWindowTagsFromMontage(Montage, CombatTags);
	}

	for (const FGameplayTag& Tag : CombatTags)
	{
		if (Tag.IsValid())
		{
			ASC->RemoveLooseGameplayTag(Tag);
		}
	}
}

void UGA_HeavyAttack::GetCombatWindowTagsFromMontage(UAnimMontage* InMontage, TArray<FGameplayTag>& OutTags)
{
	if (!InMontage)
	{
		return;
	}

	for (const FAnimNotifyEvent& NotifyEvent : InMontage->Notifies)
	{
		if (NotifyEvent.NotifyStateClass && NotifyEvent.NotifyStateClass->IsA<UAOCombatWindow>())
		{
			UAOCombatWindow* CombatWindowCDO = Cast<UAOCombatWindow>(NotifyEvent.NotifyStateClass);
			if (!CombatWindowCDO)
			{
				continue;
			}

			const FGameplayTag CombatWindowTag = CombatWindowCDO->GetCombatWindowTag();
			if (CombatWindowTag.IsValid())
			{
				OutTags.AddUnique(CombatWindowTag);
			}

			const FGameplayTag CombatingTag = CombatWindowCDO->GetCombatingTag();
			if (CombatingTag.IsValid())
			{
				OutTags.AddUnique(CombatingTag);
			}
		}
	}
}

void UGA_HeavyAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ClearCombatTags();

	if (MovementInputTask)
	{
		MovementInputTask->EndTask();
		MovementInputTask = nullptr;
	}

	if (RotationTask)
	{
		RotationTask->EndTask();
		RotationTask = nullptr;
	}

	if (WaitCombatHit)
	{
		WaitCombatHit->EndTask();
		WaitCombatHit = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_HeavyAttack::SetCharacterRotationToAttackDirection()
{
	if (!CurrentActorInfo)
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(CurrentActorInfo->AvatarActor);
	if (!Pawn)
	{
		return;
	}

	AController* Controller = Pawn->GetController();
	if (!Controller)
	{
		return;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator TargetRotation(0.0f, ControlRotation.Yaw, 0.0f);

	if (!RotationTask)
	{
		RotationTask = UAT_WaitRotateToDirection::WaitRotateToDirection(this, TargetRotation, RotationInterpSpeed);
		if (RotationTask)
		{
			RotationTask->ReadyForActivation();
		}
	}
}

UAT_WaitHeavyAttackCombatHit::UAT_WaitHeavyAttackCombatHit(const FObjectInitializer& ObjectInitializer)
{
	Weapon = nullptr;
	bTickingTask = true;
	HitResultPool.Reserve(100);
	UniqueTargetsPool.Reserve(20);
}

UAT_WaitHeavyAttackCombatHit* UAT_WaitHeavyAttackCombatHit::WaitCombatHit(UGA_HeavyAttack* OwningAbility,
	UAOWeaponInstance* WeaponInstance)
{
	UAT_WaitHeavyAttackCombatHit* MyObj = NewAbilityTask<UAT_WaitHeavyAttackCombatHit>(OwningAbility);
	MyObj->Weapon = WeaponInstance;
	MyObj->HeavyAttack = OwningAbility;
	return MyObj;
}

void UAT_WaitHeavyAttackCombatHit::Activate()
{
	Super::Activate();
	if (!Weapon.IsValid())
	{
		EndTask();
		return;
	}
	HitTargetsInActiveWindow.Reset();
	bWasHitWindowActiveLastTick = false;

	for (AActor* SpawnedActor : Weapon->GetSpawnedActors())
	{
		for (const UActorComponent* Component : SpawnedActor->GetComponents())
		{
			if (const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component))
			{
				CacheStaticMeshComponents.AddUnique(const_cast<UStaticMeshComponent*>(StaticMeshComponent));
			}
			if (const USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Component))
			{
				CacheSkeletalMeshComponents.AddUnique(const_cast<USkeletalMeshComponent*>(SkeletalMeshComponent));
			}
		}
	}

	UE_LOG(
		LogAegisOdysseyCombatTrace,
		Warning,
		TEXT("[CombatTrace][Heavy] Activate. Ability=%s StaticMeshes=%d SkeletalMeshes=%d"),
		*GetNameSafe(HeavyAttack.Get()),
		CacheStaticMeshComponents.Num(),
		CacheSkeletalMeshComponents.Num());
}

void UAT_WaitHeavyAttackCombatHit::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (!Weapon.IsValid() || !AbilitySystemComponent.IsValid() || !HeavyAttack.IsValid())
	{
		EndTask();
		return;
	}

	const bool bIsHitWindowActive = AbilitySystemComponent->GetOwnedGameplayTags().HasTagExact(HeavyAttack->ListenAttackActionTag);
	if (!bIsHitWindowActive)
	{
		if (bWasHitWindowActiveLastTick)
		{
			EndHitWindow();
		}

		bWasHitWindowActiveLastTick = false;
		return;
	}

	if (!bWasHitWindowActiveLastTick)
	{
		BeginHitWindow();
	}

	bWasHitWindowActiveLastTick = true;

	UE_LOG(
		LogAegisOdysseyCombatTrace,
		Warning,
		TEXT("[CombatTrace][Heavy] Tick active. Ability=%s StaticMeshes=%d SkeletalMeshes=%d"),
		*GetNameSafe(HeavyAttack.Get()),
		CacheStaticMeshComponents.Num(),
		CacheSkeletalMeshComponents.Num());

	HitResultPool.Empty();
	UniqueTargetsPool.Empty();
	FirstHitResultByTargetPool.Reset();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery3);

	TArray<AActor*> IgnoreActors;
	IgnoreActors.AddUnique(Ability->GetAvatarActorFromActorInfo());

	for (const UStaticMeshComponent* MeshComponent : CacheStaticMeshComponents)
	{
		const FTransform StartLocation = MeshComponent->GetSocketTransform("LightAttackStart");
		const FTransform EndLocation = MeshComponent->GetSocketTransform("LightAttackEnd");
		const FVector CurrentTraceStart = StartLocation.GetLocation();
		const FVector CurrentTraceEnd = EndLocation.GetLocation();

		TArray<FHitResult> ComponentHits;
		int32 BridgeTraceCount = 0;
		int32 DirectHitCount = 0;
		int32 BridgeHitCount = 0;
		AOHeavyAttackTracePrivate::AppendContinuousTraceHits(
			Ability,
			MeshComponent,
			CurrentTraceStart,
			CurrentTraceEnd,
			ObjectTypes,
			IgnoreActors,
			PreviousTraceStartByComponent,
			PreviousTraceEndByComponent,
			ComponentHits,
			BridgeTraceCount,
			DirectHitCount,
			BridgeHitCount);

		UE_LOG(
			LogAegisOdysseyCombatTrace,
			Warning,
			TEXT("[CombatTrace][Heavy] StaticTrace Component=%s Start=%s End=%s DirectHits=%d BridgeTraces=%d BridgeHits=%d TotalHits=%d HitActors=%s"),
			*GetNameSafe(MeshComponent),
			*CurrentTraceStart.ToCompactString(),
			*CurrentTraceEnd.ToCompactString(),
			DirectHitCount,
			BridgeTraceCount,
			BridgeHitCount,
			ComponentHits.Num(),
			*AOHeavyAttackTracePrivate::BuildHitActorSummary(ComponentHits));

		HitResultPool.Append(ComponentHits);
	}

	for (const USkeletalMeshComponent* MeshComponent : CacheSkeletalMeshComponents)
	{
		const FTransform StartLocation = MeshComponent->GetSocketTransform("LightAttackStart");
		const FTransform EndLocation = MeshComponent->GetSocketTransform("LightAttackEnd");
		const FVector CurrentTraceStart = StartLocation.GetLocation();
		const FVector CurrentTraceEnd = EndLocation.GetLocation();

		TArray<FHitResult> ComponentHits;
		int32 BridgeTraceCount = 0;
		int32 DirectHitCount = 0;
		int32 BridgeHitCount = 0;
		AOHeavyAttackTracePrivate::AppendContinuousTraceHits(
			Ability,
			MeshComponent,
			CurrentTraceStart,
			CurrentTraceEnd,
			ObjectTypes,
			IgnoreActors,
			PreviousTraceStartByComponent,
			PreviousTraceEndByComponent,
			ComponentHits,
			BridgeTraceCount,
			DirectHitCount,
			BridgeHitCount);

		UE_LOG(
			LogAegisOdysseyCombatTrace,
			Warning,
			TEXT("[CombatTrace][Heavy] SkeletalTrace Component=%s Start=%s End=%s DirectHits=%d BridgeTraces=%d BridgeHits=%d TotalHits=%d HitActors=%s"),
			*GetNameSafe(MeshComponent),
			*CurrentTraceStart.ToCompactString(),
			*CurrentTraceEnd.ToCompactString(),
			DirectHitCount,
			BridgeTraceCount,
			BridgeHitCount,
			ComponentHits.Num(),
			*AOHeavyAttackTracePrivate::BuildHitActorSummary(ComponentHits));

		HitResultPool.Append(ComponentHits);
	}

	ProcessHitsBatch();
}

void UAT_WaitHeavyAttackCombatHit::ProcessHitsBatch()
{
	ENetMode NetMode = NM_Standalone;
	if (Ability && Ability->GetWorld())
	{
		NetMode = Ability->GetWorld()->GetNetMode();
		if (NetMode == NM_Client)
		{
			return;
		}
	}

	if (HitResultPool.Num() == 0)
	{
		UE_LOG(
			LogAegisOdysseyCombatTrace,
			Warning,
			TEXT("[CombatTrace][Heavy] No hits collected. Ability=%s"),
			*GetNameSafe(HeavyAttack.Get()));
		return;
	}

	for (const FHitResult& Hit : HitResultPool)
	{
		AActor* Target = Hit.GetActor();
		if (Target && Target->Implements<UCombatInterface>())
		{
			const TObjectKey<AActor> TargetKey(Target);
			if (!FirstHitResultByTargetPool.Contains(TargetKey))
			{
				FirstHitResultByTargetPool.Add(TargetKey, Hit);
				UniqueTargetsPool.Add(Target);
			}
		}
	}

	UE_LOG(
		LogAegisOdysseyCombatTrace,
		Warning,
		TEXT("[CombatTrace][Heavy] RawHits=%d UniqueTargets=%d WindowTrackedTargets=%d Ability=%s"),
		HitResultPool.Num(),
		UniqueTargetsPool.Num(),
		HitTargetsInActiveWindow.Num(),
		*GetNameSafe(HeavyAttack.Get()));

	TArray<TSubclassOf<UGameplayEffect>> MetaEffects;
	if (Weapon.IsValid())
	{
		if (UAOWeaponDefinition* WeaponDef = Cast<UAOWeaponDefinition>(Weapon->GetItemCDO()))
		{
			const TArray<TObjectPtr<UAOAbilitySet>>& AbilitySets = WeaponDef->GetAbilitySetsToGrant();
			for (const TObjectPtr<UAOAbilitySet>& AbilitySet : AbilitySets)
			{
				if (!AbilitySet)
				{
					continue;
				}

				for (const FAOAbilitySet_MetaGameplayEffect& MetaEffect : AbilitySet->MetaGameplayEffects)
				{
					if (MetaEffect.GameplayEffect)
					{
						MetaEffects.Add(MetaEffect.GameplayEffect);
					}
				}
			}
		}
	}

	for (AActor* Target : UniqueTargetsPool)
	{
		if (!HeavyAttack.IsValid())
		{
			continue;
		}

		const TObjectKey<AActor> TargetKey(Target);
		if (HitTargetsInActiveWindow.Contains(TargetKey))
		{
			UE_LOG(
				LogAegisOdysseyCombatTrace,
				Warning,
				TEXT("[CombatTrace][Heavy] Skip duplicate target in same window. Ability=%s Target=%s"),
				*GetNameSafe(HeavyAttack.Get()),
				*GetNameSafe(Target));
			continue;
		}

		HitTargetsInActiveWindow.Add(TargetKey);

		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Target))
		{
			FAttackedInfo AttackedInfo;
			AttackedInfo.SourceASC = AbilitySystemComponent.Get();
			AttackedInfo.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
			AttackedInfo.SourceObject = Weapon.IsValid() ? StaticCast<UObject*>(Weapon.Get()) : StaticCast<UObject*>(HeavyAttack.Get());
			AttackedInfo.EffectCauser = GetAvatarActor();
			AttackedInfo.AttackTag = HeavyAttack.IsValid() ? HeavyAttack->AttackTag : FGameplayTag();
			AttackedInfo.MetaGameplayEffects = MetaEffects;
			if (const FHitResult* FirstHitResult = FirstHitResultByTargetPool.Find(TObjectKey<AActor>(Target)))
			{
				AttackedInfo.HitResult = *FirstHitResult;
			}

			if (Weapon.IsValid())
			{
				if (const UAOWeaponDefinition* WeaponDef = Cast<UAOWeaponDefinition>(Weapon->GetItemCDO()))
				{
					AttackedInfo.WeaponTag = WeaponDef->GetWeaponTag();
					AttackedInfo.DamageTypeTags = WeaponDef->GetDamageTypeTags();
				}
			}

			CombatInterface->ApplyDamageToTarget(AttackedInfo);
		}
	}
}

void UAT_WaitHeavyAttackCombatHit::OnDestroy(bool bInOwnerFinished)
{
	EndHitWindow();
	Super::OnDestroy(bInOwnerFinished);
	CacheStaticMeshComponents.Reset();
	CacheSkeletalMeshComponents.Reset();
	PreviousTraceStartByComponent.Reset();
	PreviousTraceEndByComponent.Reset();
	FirstHitResultByTargetPool.Reset();
	HitTargetsInActiveWindow.Reset();
}

void UAT_WaitHeavyAttackCombatHit::BeginHitWindow()
{
	HitTargetsInActiveWindow.Reset();
	PreviousTraceStartByComponent.Reset();
	PreviousTraceEndByComponent.Reset();
	UE_LOG(
		LogAegisOdysseyCombatTrace,
		Warning,
		TEXT("[CombatTrace][Heavy] BeginHitWindow. Ability=%s"),
		*GetNameSafe(HeavyAttack.Get()));
}

void UAT_WaitHeavyAttackCombatHit::EndHitWindow()
{
	HitTargetsInActiveWindow.Reset();
	PreviousTraceStartByComponent.Reset();
	PreviousTraceEndByComponent.Reset();
	UE_LOG(
		LogAegisOdysseyCombatTrace,
		Warning,
		TEXT("[CombatTrace][Heavy] EndHitWindow. Ability=%s"),
		*GetNameSafe(HeavyAttack.Get()));
}

UAOWeaponInstance* UGA_HeavyAttack::GetAOWeaponInstance() const
{
	if (AAOCharacter* SourceCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo()))
	{
		UAOWeaponManagerComponent* WeaponManagerComponent = SourceCharacter->GetComponentByClass<UAOWeaponManagerComponent>();
		if (!WeaponManagerComponent)
		{
			return nullptr;
		}

		return Cast<UAOWeaponInstance>(WeaponManagerComponent->GetCurrentWeaponInstance());
	}

	return nullptr;
}

UAOWeaponDefinition* UGA_HeavyAttack::GetAOWeaponDefinition() const
{
	if (AAOCharacter* SourceCharacter = Cast<AAOCharacter>(GetAvatarActorFromActorInfo()))
	{
		UAOWeaponManagerComponent* WeaponManagerComponent = SourceCharacter->GetComponentByClass<UAOWeaponManagerComponent>();
		if (!WeaponManagerComponent)
		{
			return nullptr;
		}

		if (UAOWeaponInstance* Weapon = Cast<UAOWeaponInstance>(WeaponManagerComponent->GetCurrentWeaponInstance()))
		{
			return Cast<UAOWeaponDefinition>(Weapon->GetItemCDO());
		}
	}

	return nullptr;
}
