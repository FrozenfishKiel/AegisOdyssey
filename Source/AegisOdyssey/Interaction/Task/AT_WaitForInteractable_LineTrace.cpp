// Fill out your copyright notice in the Description page of Project Settings.


#include "AT_WaitForInteractable_LineTrace.h"

#include "AegisOdyssey/Interaction/InteractionStatics.h"
#include "Engine/Engine.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AT_WaitForInteractable_LineTrace)

UAT_WaitForInteractable_LineTrace* UAT_WaitForInteractable_LineTrace::WaitForInteractableTarget_SingleLineTrace(
	UGameplayAbility* OwingAbility, FCollisionProfileName TraceProfile,
	FGameplayAbilityTargetingLocationInfo StartLocation, float InteractionScanRange, float InteractionScanRate,
	bool bShowDebug)
{
	UAT_WaitForInteractable_LineTrace* MyObj = NewAbilityTask<UAT_WaitForInteractable_LineTrace>(OwingAbility);
	MyObj->InteractionScanRange = InteractionScanRange;
	MyObj->InteractionScanRate = InteractionScanRate;
	MyObj->StartLocation = StartLocation;
	MyObj->bShowDebug = bShowDebug;
	MyObj->TraceProfile = TraceProfile;
	return MyObj;
}

void UAT_WaitForInteractable_LineTrace::Activate()
{
	SetWaitingOnAvatar();

	UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(TimerHandle,this,&ThisClass::PerformTrace,InteractionScanRate , true);
}

void UAT_WaitForInteractable_LineTrace::OnDestroy(bool bInOwnerFinished)
{
	UWorld* World = GetWorld();
	if(World)
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}
	Super::OnDestroy(bInOwnerFinished);
}

void UAT_WaitForInteractable_LineTrace::PerformTrace()
{
	AActor* AvatarActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	if (!AvatarActor) return;

	UWorld* World = GetWorld();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(AvatarActor);  //添加忽略的Actor对象（释放技能的Actor）

	const bool bTraceComplex = false;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(UAT_WaitForInteractable_LineTrace), bTraceComplex);
	Params.AddIgnoredActors(ActorsToIgnore);

	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
	FVector TraceEnd;

	AimWithPlayerController(AvatarActor , Params , TraceStart , InteractionScanRange , OUT  TraceEnd);  //从摄像机的跟踪线

	FHitResult OutHitResult;
	LineTrace(OutHitResult, World, TraceStart, TraceEnd, TraceProfile.Name, Params);  //角色身上的跟踪线


	TArray<TScriptInterface<IInteractableTarget>> InteractableTargets;
	//加载并添加该对象所有的InteractableTarget接口
	UInteractionStatics::AppendInteractableTargetsFromHitResult(OutHitResult, InteractableTargets);

	UpdateInteractableOptions(InteractableTargets);

#if ENABLE_DRAW_DEBUG
	if (bShowDebug)
	{
		FColor DebugColor = OutHitResult.bBlockingHit ? FColor::Red : FColor::Green;
		if (OutHitResult.bBlockingHit)
		{
			DrawDebugLine(World, TraceStart, OutHitResult.Location, DebugColor, false, InteractionScanRate);
			//DrawDebugLine(World, TraceStart, OUT TraceEnd, DebugColor, false, InteractionScanRate);

			DrawDebugSphere(World, OutHitResult.Location, 5, 16, DebugColor, false, InteractionScanRate);
		}
		else
		{
			DrawDebugLine(World, TraceStart, TraceEnd, DebugColor, false, InteractionScanRate);
		}
	}
#endif // ENABLE_DRAW_DEBUG
}
