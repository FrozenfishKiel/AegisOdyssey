// Fill out your copyright notice in the Description page of Project Settings.


#include "AT_WaitForInteractableTargets.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AegisOdyssey/Interaction/InteractableTarget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AT_WaitForInteractableTargets)

UAT_WaitForInteractableTargets::UAT_WaitForInteractableTargets(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	
}

void UAT_WaitForInteractableTargets::LineTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Start,
	const FVector& End, FName ProfileName, const FCollisionQueryParams Params)
{
	check(World);

	OutHitResult = FHitResult();
	TArray<FHitResult> HitResults;
	World->LineTraceMultiByProfile(HitResults , Start, End, ProfileName, Params);

	OutHitResult.TraceStart = Start;
	OutHitResult.TraceEnd = End;

	//如果碰撞的数量大于0则只拿第一次碰撞的对象
	if (HitResults.Num() > 0)
	{
		OutHitResult = HitResults[0];
	}
}

/*将摄像机射线裁剪到能力范围球体上，并返回射线与球体的交点（如果存在）。主要用于判断从摄像机位置沿指定方向发射的射线是否与一个球体相交，并计算交点位置。*/
bool UAT_WaitForInteractableTargets::ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection,
	FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition)
{
	FVector CameraToCenter = AbilityCenter - CameraLocation;
	float DotToCenter = FVector::DotProduct(CameraToCenter, CameraDirection);
	if (DotToCenter >= 0)		//If this fails, we're pointed away from the center, but we might be inside the sphere and able to find a good exit point.
	{
		float DistanceSquared = CameraToCenter.SizeSquared() - (DotToCenter * DotToCenter);
		float RadiusSquared = (AbilityRange * AbilityRange);
		if (DistanceSquared <= RadiusSquared)
		{
			float DistanceFromCamera = FMath::Sqrt(RadiusSquared - DistanceSquared);
			float DistanceAlongRay = DotToCenter + DistanceFromCamera;						//Subtracting instead of adding will get the other intersection point
			ClippedPosition = CameraLocation + (DistanceAlongRay * CameraDirection);		//Cam aim point clipped to range sphere
			return true;
		}
	}
	return false;
}

/*实现基于玩家控制器的瞄准系统。它结合了摄像机射线裁剪和碰撞检测，计算出最终的瞄准终点位置*/
void UAT_WaitForInteractableTargets::AimWithPlayerController(const AActor* InSourceActor,
	FCollisionQueryParams Params, const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd,
	bool bIgnorePitch) const
{
	if (!Ability) // Server and launching client only
	{
		return;
	}
	//const FGameplayAbilityActorInfo* SourceActor = Ability->GetCurrentActorInfo();
	APlayerController* PC = Cast<APlayerController>( Ability->GetAvatarActorFromActorInfo()->GetInstigatorController());
	check(PC);

	FVector ViewStart;
	FRotator ViewRot;
	PC->GetPlayerViewPoint(ViewStart, ViewRot);

	const FVector ViewDir = ViewRot.Vector();
	FVector ViewEnd = ViewStart + (ViewDir * MaxRange);

	ClipCameraRayToAbilityRange(ViewStart, ViewDir, TraceStart, MaxRange, ViewEnd);

	FHitResult HitResult;
	LineTrace(HitResult, InSourceActor->GetWorld(), ViewStart, ViewEnd, TraceProfile.Name, Params);

	const bool bUseTraceResult = HitResult.bBlockingHit && (FVector::DistSquared(TraceStart, HitResult.Location) <= (MaxRange * MaxRange));

	const FVector AdjustedEnd = (bUseTraceResult) ? HitResult.Location : ViewEnd;

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
			// Convert to angles and use original pitch
			const FRotator OriginalAimRot = OriginalAimDir.Rotation();

			FRotator AdjustedAimRot = AdjustedAimDir.Rotation();
			AdjustedAimRot.Pitch = OriginalAimRot.Pitch;

			AdjustedAimDir = AdjustedAimRot.Vector();
		}
	}

	OutTraceEnd = TraceStart + (AdjustedAimDir * MaxRange);
}

//传入对象Actor或ActorComp的所有InteractableTarget接口
void UAT_WaitForInteractableTargets::UpdateInteractableOptions(
	const TArray<TScriptInterface<IInteractableTarget>>& InteractableTargets)
{
	TArray<FInteractionOption> NewOptions;

	for (TScriptInterface<IInteractableTarget> InteractableTarget : InteractableTargets)
	{
		TArray<FInteractionOption> TempOptions;
		FInteractionOptionBuilder InteractBuilder(InteractableTarget , TempOptions);
		InteractableTarget->GatherInteractionOptions(InteractBuilder);  //加载并获取TempOptions

		for (FInteractionOption& Option : TempOptions)
		{
			FGameplayAbilitySpec* InteractionAbilitySpec = nullptr;
			//获取交互的Actor身上的ASC

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

			if (InteractionAbilitySpec)
			{
				if (InteractionAbilitySpec->Ability->CanActivateAbility(InteractionAbilitySpec->Handle , AbilitySystemComponent->AbilityActorInfo.Get()))
				{
					NewOptions.Add(Option);
				}
			}
		}
	}

	bool bOptionChanged = false;
	//检查两个数组是否完全一致
	if (NewOptions.Num() == CurrentOptions.Num())
	{
		NewOptions.Sort();  //快速排序

		for (int OptionIndex = 0 ; OptionIndex < NewOptions.Num() ; OptionIndex++)
		{
			const FInteractionOption& NewOption = NewOptions[OptionIndex];
			const FInteractionOption& CurrentOption = CurrentOptions[OptionIndex];

			if (NewOption != CurrentOption)
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
