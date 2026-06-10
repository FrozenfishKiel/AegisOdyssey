#include "AOCombatFloatingTextComponent.h"

#include "AegisOdyssey/UI/ViewModel/AOCombatFeedbackBlueprintLibrary.h"
#include "AegisOdyssey/UI/ViewModel/AOCombatFeedbackViewData.h"
#include "AOCombatFloatingTextWidget.h"
#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCombatFloatingTextComponent)

UAOCombatFloatingTextComponent::UAOCombatFloatingTextComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAOCombatFloatingTextComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (UWidgetComponent* WidgetComponent : FloatingTextWidgetPool)
	{
		if (WidgetComponent != nullptr)
		{
			WidgetComponent->SetVisibility(false, true);
			WidgetComponent->SetHiddenInGame(true);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UAOCombatFloatingTextComponent::CleanupExpiredWidgets()
{
	for (int32 Index = FloatingTextWidgetPool.Num() - 1; Index >= 0; --Index)
	{
		UWidgetComponent* WidgetComponent = FloatingTextWidgetPool[Index];
		if (WidgetComponent == nullptr || !IsValid(WidgetComponent))
		{
			FloatingTextWidgetPool.RemoveAt(Index);
		}
	}
}

UWidgetComponent* UAOCombatFloatingTextComponent::AcquireFloatingTextWidgetComponent(const FVector& WorldAnchorLocation)
{
	if (GetOwner() == nullptr || GetWorld() == nullptr || GetWorld()->IsNetMode(NM_DedicatedServer) || FloatingTextWidgetClass == nullptr)
	{
		return nullptr;
	}

	CleanupExpiredWidgets();

	if (MaxActiveFloatingTexts > 0 && FloatingTextWidgetPool.Num() >= MaxActiveFloatingTexts)
	{
		UWidgetComponent* ReusedWidgetComponent = FloatingTextWidgetPool[0];
		FloatingTextWidgetPool.RemoveAt(0);
		if (ReusedWidgetComponent != nullptr)
		{
			FloatingTextWidgetPool.Add(ReusedWidgetComponent);
			ReusedWidgetComponent->SetWorldLocation(WorldAnchorLocation);
			ReusedWidgetComponent->SetVisibility(true, true);
			ReusedWidgetComponent->SetHiddenInGame(false);
			ReusedWidgetComponent->InitWidget();
			return ReusedWidgetComponent;
		}
	}

	UWidgetComponent* WidgetComponent = NewObject<UWidgetComponent>(GetOwner());
	if (WidgetComponent == nullptr)
	{
		return nullptr;
	}

	GetOwner()->AddInstanceComponent(WidgetComponent);
	WidgetComponent->SetWidgetSpace(WidgetSpace);
	WidgetComponent->SetDrawAtDesiredSize(false);
	WidgetComponent->SetDrawSize(WidgetDrawSize);
	WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WidgetComponent->SetGenerateOverlapEvents(false);
	WidgetComponent->SetWidgetClass(FloatingTextWidgetClass);
	WidgetComponent->RegisterComponent();

	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh())
		{
			WidgetComponent->AttachToComponent(MeshComponent, FAttachmentTransformRules::KeepWorldTransform);
		}
	}
	else if (USceneComponent* RootComponent = GetOwner()->GetRootComponent())
	{
		WidgetComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	}

	WidgetComponent->SetWorldLocation(WorldAnchorLocation);
	WidgetComponent->SetVisibility(true, true);
	WidgetComponent->SetHiddenInGame(false);
	WidgetComponent->InitWidget();

	FloatingTextWidgetPool.Add(WidgetComponent);
	return WidgetComponent;
}

FVector UAOCombatFloatingTextComponent::ResolveFloatingTextWorldLocation(const FAOCombatFeedbackViewData& FeedbackViewData) const
{
	if (FeedbackViewData.HitResult.bBlockingHit || FeedbackViewData.HitResult.bStartPenetrating || !FeedbackViewData.HitResult.ImpactPoint.IsNearlyZero())
	{
		return FeedbackViewData.HitResult.ImpactPoint;
	}

	if (AActor* TargetActor = FeedbackViewData.Target.Get())
	{
		return TargetActor->GetActorLocation() + FallbackWorldOffset;
	}

	if (const AActor* OwnerActor = GetOwner())
	{
		return OwnerActor->GetActorLocation() + FallbackWorldOffset;
	}

	return FallbackWorldOffset;
}

void UAOCombatFloatingTextComponent::DisplayWorldCombatFeedback(const FAOCombatFeedbackViewData& FeedbackViewData)
{
	if (!FeedbackViewData.bShouldEnqueueForWorldFloatingText)
	{
		return;
	}

	const FVector WorldAnchorLocation = ResolveFloatingTextWorldLocation(FeedbackViewData);
	UWidgetComponent* WidgetComponent = AcquireFloatingTextWidgetComponent(WorldAnchorLocation);
	if (WidgetComponent == nullptr)
	{
		return;
	}

	if (UAOCombatFloatingTextWidget* FloatingTextWidget = Cast<UAOCombatFloatingTextWidget>(WidgetComponent->GetUserWidgetObject()))
	{
		FloatingTextWidget->HandleWorldCombatFeedback(
			FeedbackViewData,
			UAOCombatFeedbackBlueprintLibrary::BuildRecommendedCombatText(FeedbackViewData),
			WorldAnchorLocation);
	}
}
