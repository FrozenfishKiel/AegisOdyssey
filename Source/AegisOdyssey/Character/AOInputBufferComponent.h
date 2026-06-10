// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "GameplayTagContainer.h"
#include "AOInputBufferComponent.generated.h"

enum EInputType : uint8;
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStartInputBuffer,const FGameplayTag,const EInputType);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPressInputBuffer,const FGameplayTag,const EInputType);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnReleaseInputBuffer,const FGameplayTag,const EInputType);
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AEGISODYSSEY_API UAOInputBufferComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAOInputBufferComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Input Buffer")
	void SetBufferedInput(const FGameplayTag& InputTag, EInputType InputType);
	
	UFUNCTION(BlueprintPure, Category = "Input Buffer")
	inline FGameplayTag GetBufferedInput() const;

	UFUNCTION(BlueprintPure, Category = "Input Buffer")
	inline EInputType GetBufferedInputType() const;
	
	UFUNCTION(BlueprintCallable, Category = "Input Buffer")
	void ClearBufferedInput();

	UFUNCTION(BlueprintPure, Category = "Input Buffer")
	bool IsBufferedInputValid() const;  //检查预输入是否过期

	UFUNCTION(BlueprintCallable, Category = "Input Buffer")
	bool TriggerBufferedInput();  //触发预输入要激活的东西

	static UAOInputBufferComponent* FindOInputBufferComponent(const AActor* Actor){return Actor ? Actor->FindComponentByClass<UAOInputBufferComponent>() : nullptr;}

	FOnStartInputBuffer OnStartInputBuffer;
	FOnPressInputBuffer OnPressInputBuffer;
	FOnReleaseInputBuffer OnReleaseInputBuffer;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

protected:
	
	float InputTimestamp;  //输入时间戳，用于记录玩家在什么时候按下的按键
	FGameplayTag SaveInputTag;  //储存的预输入标签，用于输入触发
	UPROPERTY(EditAnywhere , BlueprintReadWrite , Category="Config")
	float BufferDuration = 0.5f;  //预输入有效时间
	EInputType SaveInputType;  //输入状态，是否是长按，松开
};

