// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AegisOdyssey/Character/AOHeroComponent.h"
#include "AegisOdyssey/StateTree/AOStateTreeComponentBase.h"
#include "AOCombatStateTree.generated.h"

enum EInputType : uint8;

USTRUCT(BlueprintType)
struct FCombatStateTreeInputEvent
{
	GENERATED_BODY()

	FCombatStateTreeInputEvent()
		: 
		InputType(EInputType::None)
	{
	}

	FCombatStateTreeInputEvent(const FGameplayTag InInputTag, const EInputType InInputType)
		: 
		InputType(InInputType)
	{
	}

	UPROPERTY(EditAnywhere, Category = "CombatStateTree")
	TEnumAsByte<EInputType> InputType;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent),DisplayName = "AOChainComboStateTree")
class AEGISODYSSEY_API UAOCombatStateTree : public UAOStateTreeComponentBase
{
	GENERATED_BODY()

public:
	UAOCombatStateTree();

protected:
	virtual void BeginPlay() override;
	virtual void CallStateTreeToSentEvent(const FGameplayTag InTargetTag,const EInputType InInputType) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnRegister() override;
	virtual void InitializeComponent() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	FDelegateHandle OnPressInputLoadHandle;
	FDelegateHandle OnReleaseInputLoadHandle;
	FDelegateHandle OnStartInputLoadHandle;

	FDelegateHandle OnPressInputBufferHandle;
	FDelegateHandle OnReleaseInputBufferHandle;
	FDelegateHandle OnStartInputBufferHandle;
};
