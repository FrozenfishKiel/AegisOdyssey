// Fill out your copyright notice in the Description page of Project Settings.


#include "AT_WaitForInteractableTargets.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AegisOdyssey/Interaction/InteractableTarget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AT_WaitForInteractableTargets)

UAT_WaitForInteractableTargets::UAT_WaitForInteractableTargets(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAT_WaitForInteractableTargets::LineTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Start,
	const FVector& End, FName ProfileName, const FCollisionQueryParams Params)
{
	check(World);

	OutHitResult = FHitResult();
	TArray<FHitResult> HitResults;
	World->LineTraceMultiByProfile(HitResults, Start, End, ProfileName, Params);

	OutHitResult.TraceStart = Start;
	OutHitResult.TraceEnd = End;

	// 如果发生了多个碰撞，则只取第一个阻挡命中的对象。
	if (HitResults.Num() > 0)
	{
		OutHitResult = HitResults[0];
	}
}

bool UAT_WaitForInteractableTargets::ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection,
	FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition)
{
	FVector CameraToCenter = AbilityCenter - CameraLocation;
	float DotToCenter = FVector::DotProduct(CameraToCenter, CameraDirection);
	if (DotToCenter >= 0)
	{
		float DistanceSquared = CameraToCenter.SizeSquared() - (DotToCenter * DotToCenter);
		float RadiusSquared = (AbilityRange * AbilityRange);
		if (DistanceSquared <= RadiusSquared)
		{
			float DistanceFromCamera = FMath::Sqrt(RadiusSquared - DistanceSquared);
			float DistanceAlongRay = DotToCenter + DistanceFromCamera;
			ClippedPosition = CameraLocation + (DistanceAlongRay * CameraDirection);
			return true;
		}
	}
	return false;
}

void UAT_WaitForInteractableTargets::AimWithPlayerController(const AActor* InSourceActor,
	FCollisionQueryParams Params, const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd,
	bool bIgnorePitch) const
{
	if (!Ability)
	{
		return;
	}
	APawn* AvatarPawn = Cast<APawn>(Ability->GetAvatarActorFromActorInfo());
	APlayerController* PC = AvatarPawn ? Cast<APlayerController>(AvatarPawn->GetController()) : nullptr;
	

	FVector ViewStart;
	FRotator ViewRot;
	if (PC)
	{
		PC->GetPlayerViewPoint(ViewStart, ViewRot);
	}
	const FVector ViewDir = ViewRot.Vector();
	FVector ViewEnd = ViewStart + (ViewDir * MaxRange);

	ClipCameraRayToAbilityRange(ViewStart, ViewDir, TraceStart, MaxRange, ViewEnd);

	FHitResult HitResult;
	LineTrace(HitResult, InSourceActor->GetWorld(), ViewStart, ViewEnd, TraceProfile.Name, Params);

	const bool bUseTraceResult = HitResult.bBlockingHit && (FVector::DistSquared(TraceStart, HitResult.Location) <= (MaxRange * MaxRange));
	const FVector AdjustedEnd = bUseTraceResult ? HitResult.Location : ViewEnd;

	FVector AdjustedAimDir = (AdjustedEnd - TraceStart).GetSafeNormal();
	if (AdjustedAimDir.IsZero())
	{
		AdjustedAimDir = ViewDir;
	}

	if (!bTraceAffectsAimPitch && bUseTraceResult)
	{
		FVector OriginalAimDir = (ViewEnd - TraceStart).GetSafeNormal();

		if (!OriginalAimDir.IsZero())
		{
			const FRotator OriginalAimRot = OriginalAimDir.Rotation();

			FRotator AdjustedAimRot = AdjustedAimDir.Rotation();
			AdjustedAimRot.Pitch = OriginalAimRot.Pitch;

			AdjustedAimDir = AdjustedAimRot.Vector();
		}
	}

	OutTraceEnd = TraceStart + (AdjustedAimDir * MaxRange);
}

// 传入对象 Actor 或 ActorComponent 上实现的全部 InteractableTarget 接口，
// 统一收集它们当前暴露出的交互选项。
void UAT_WaitForInteractableTargets::UpdateInteractableOptions(
	const TArray<TScriptInterface<IInteractableTarget>>& InteractableTargets)
{
	TArray<FInteractionOption> NewOptions;

	for (TScriptInterface<IInteractableTarget> InteractableTarget : InteractableTargets)
	{
		TArray<FInteractionOption> TempOptions;
		FInteractionOptionBuilder InteractBuilder(InteractableTarget, TempOptions);
		InteractableTarget->GatherInteractionOptions(InteractBuilder);

		for (FInteractionOption& Option : TempOptions)
		{
			FGameplayAbilitySpec* InteractionAbilitySpec = nullptr;

			// 旧拾取链依赖扫描阶段给交互选项补上能力句柄。
			if (Option.TargetAbilitySystem && Option.TargetInteractionAbilityHandle.IsValid())
			{
				InteractionAbilitySpec = Option.TargetAbilitySystem->FindAbilitySpecFromHandle(Option.TargetInteractionAbilityHandle);
			}
			else if (Option.InteractionAbilityToGrant)
			{
				InteractionAbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(Option.InteractionAbilityToGrant);
				if (InteractionAbilitySpec)
				{
					Option.TargetAbilitySystem = AbilitySystemComponent.Get();
					Option.TargetInteractionAbilityHandle = InteractionAbilitySpec->Handle;
				}
			}

			// 没有旧能力回退需求的对象，直接暴露给统一交互链即可。
			if (!Option.InteractionAbilityToGrant)
			{
				NewOptions.Add(Option);
				continue;
			}

			// 旧链对象保留可激活性筛选，避免扫描到不能执行的交互项。
			if (InteractionAbilitySpec &&
				InteractionAbilitySpec->Ability &&
				InteractionAbilitySpec->Ability->CanActivateAbility(
					InteractionAbilitySpec->Handle,
					AbilitySystemComponent->AbilityActorInfo.Get()))
			{
				NewOptions.Add(Option);
			}
		}
	}

	bool bOptionChanged = false;
	if (NewOptions.Num() == CurrentOptions.Num())
	{
		NewOptions.Sort();
		for (int32 OptionIndex = 0; OptionIndex < NewOptions.Num(); ++OptionIndex)
		{
			const FInteractionOption& NewOption = NewOptions[OptionIndex];
			const FInteractionOption& ExistingOption = CurrentOptions[OptionIndex];

			if (NewOption != ExistingOption)
			{
				bOptionChanged = true;
				break;
			}
		}
	}
	else
	{
		bOptionChanged = true;
	}

	if (bOptionChanged)
	{
		CurrentOptions = NewOptions;
		InteractableObjectsChanged.Broadcast(CurrentOptions);
	}
}
