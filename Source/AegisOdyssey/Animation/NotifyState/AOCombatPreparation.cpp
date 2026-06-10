// Fill out your copyright notice in the Description page of Project Settings.


#include "AOCombatPreparation.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/Character/AOCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCombatPreparation)

UAOCombatPreparation::UAOCombatPreparation()
{
	CombatPreparationTag = FGameplayTag();
}

void UAOCombatPreparation::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                       float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) return;

	AAOCharacter* OwnerCharacter = Cast<AAOCharacter>(OwnerActor);
	if (!OwnerCharacter) return;

	// 获取AbilitySystemComponent
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerCharacter);
	if (!ASC)
	{
		//UE_LOG(LogTemp, Warning, TEXT("AOCombatWindow: ASC is null"));
		return;
	}
	
	// 添加连招窗口标签
	if (CombatPreparationTag.IsValid())
	{
		ASC->AddLooseGameplayTag(CombatPreparationTag);
		//UE_LOG(LogTemp, Log, TEXT("AOCombatWindow: Added CombatWindowTag to %s"), *AOCharacter->GetName());
	}
}

void UAOCombatPreparation::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	// 获取拥有骨骼网格组件的Actor
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("AOCombatWindow: OwnerActor is null"));
		return;
	}
	
	// 尝试转换为AOCharacter
	AAOCharacter* AOCharacter = Cast<AAOCharacter>(OwnerActor);
	if (!AOCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("AOCombatWindow: OwnerActor is not AOCharacter"));
		return;
	}
	
	// 获取AbilitySystemComponent
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AOCharacter);
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("AOCombatWindow: ASC is null"));
		return;
	}
	
	// 移除连招窗口标签
	if (CombatPreparationTag.IsValid())
	{
		ASC->RemoveLooseGameplayTag(CombatPreparationTag);
		UE_LOG(LogTemp, Log, TEXT("AOCombatWindow: Removed CombatWindowTag from %s"), *AOCharacter->GetName());
	}
}
