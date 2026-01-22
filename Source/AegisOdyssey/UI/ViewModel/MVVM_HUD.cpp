// Fill out your copyright notice in the Description page of Project Settings.


#include "MVVM_HUD.h"

#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MVVM_HUD)

UMVVM_HUD::UMVVM_HUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UMVVM_HUD::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true; // 启用推送模型
	Params.RepNotifyCondition = REPNOTIFY_Always;
	DOREPLIFETIME_WITH_PARAMS(ThisClass,Health,Params);
	DOREPLIFETIME_WITH_PARAMS(ThisClass,MaxHealth,Params);
}

void UMVVM_HUD::SetPlayerViewModelParams(const FPlayerMainHUDViewModelParams& params)
{
	if (params.ASC != nullptr)
	{
		PlayerViewModelParams.ASC = params.ASC;
	}
	if (params.PC != nullptr)
	{
		PlayerViewModelParams.PC = params.PC;
	}
	if (params.PS != nullptr) 
	{
		PlayerViewModelParams.PS = params.PS;
	}
	OnParamSet();
}

void UMVVM_HUD::OnParamSet_Implementation()
{
	UAOAbilitySystem* SourceASC =  GetSourceASC();
	if (SourceASC== nullptr) return;

	
	const UAOHealthAttributeSet* HealthAttributeSet = PlayerViewModelParams.ASC->GetSet<UAOHealthAttributeSet>();
	if (!HealthAttributeSet) return;
	SourceASC->GetGameplayAttributeValueChangeDelegate(HealthAttributeSet->GetMaxHealthAttribute()).AddLambda([this]
		(const FOnAttributeChangeData& Data)
	{
		SetMaxHealth(Data.NewValue);
	});
	
	SourceASC->GetGameplayAttributeValueChangeDelegate(HealthAttributeSet->GetHealthAttribute()).AddLambda([this]
		(const FOnAttributeChangeData& Data)
	{
		SetHealth(Data.NewValue);
	});

	// 立即更新属性状态，防止遗漏状态
	if (HealthAttributeSet->GetMaxHealth() > 0)
	{
		SetMaxHealth(HealthAttributeSet->GetMaxHealth());
	}

	if (HealthAttributeSet->GetHealth() > 0.f)
	{
		SetHealth(HealthAttributeSet->GetHealth());
	}

}


void UMVVM_HUD::SetHealth(const float InHealth)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(Health , InHealth))
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Health, this);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	}
}

void UMVVM_HUD::SetMaxHealth(const float InMaxHealth)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(MaxHealth , InMaxHealth))
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MaxHealth, this);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	}
}

float UMVVM_HUD::GetHealthPercent() const
{
	if (MaxHealth != 0.f)
	{
		return Health / MaxHealth;
	}
	return 0.f;
}

UAOAbilitySystem* UMVVM_HUD::GetSourceASC() const
{
	if (PlayerViewModelParams.ASC == nullptr) return nullptr;
	return Cast<UAOAbilitySystem> (PlayerViewModelParams.ASC);
}

AAOPlayerController* UMVVM_HUD::GetSourcePC() const
{
	if (PlayerViewModelParams.PC == nullptr) return nullptr;
	return Cast<AAOPlayerController> (PlayerViewModelParams.PC);
}

AAOPlayerState* UMVVM_HUD::GetSourcePS() const
{
	if (PlayerViewModelParams.PS == nullptr) return nullptr;
	return Cast<AAOPlayerState> (PlayerViewModelParams.PS);
}

void UMVVM_HUD::OnRep_Health()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	UE_LOG(LogTemp, Warning, TEXT("客户端获取Health"));

}

void UMVVM_HUD::OnRep_MaxHealth()
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent);
	UE_LOG(LogTemp, Warning, TEXT("客户端获取MaxHealth"));
}
