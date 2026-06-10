#include "AOCombatWindow.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AegisOdyssey/Character/AOCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOCombatWindow)

UAOCombatWindow::UAOCombatWindow()
{
}

void UAOCombatWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// 动画通知只挂在角色网格上，这里先把真正的战斗拥有者取回来。
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("AOCombatWindow: OwnerActor is null"));
		return;
	}

	// 当前战斗窗口逻辑只服务 AOCharacter，其他拥有者直接忽略。
	AAOCharacter* AOCharacter = Cast<AAOCharacter>(OwnerActor);
	if (!AOCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("AOCombatWindow: OwnerActor is not AOCharacter"));
		return;
	}

	// CombatWindowTag 统一挂在角色 ASC 上。
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AOCharacter);
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("AOCombatWindow: ASC is null"));
		return;
	}

	// 开窗标签仍然保留，供现有连招/输入放行逻辑继续使用。
	if (CombatWindowTag.IsValid())
	{
		ASC->AddLooseGameplayTag(CombatWindowTag);
		UE_LOG(LogTemp, Log, TEXT("AOCombatWindow: Added CombatWindowTag to %s"), *AOCharacter->GetName());
	}
}

void UAOCombatWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	// 结束窗口时走同一条拥有者链，保证开始/结束使用同一份上下文。
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	// 当前战斗窗口逻辑只服务 AOCharacter，其他拥有者直接忽略。
	AAOCharacter* AOCharacter = Cast<AAOCharacter>(OwnerActor);
	if (!AOCharacter)
	{
		return;
	}

	// CombatWindowTag 统一挂在角色 ASC 上。
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AOCharacter);
	if (!ASC)
	{
		return;
	}

	// 关窗时把旧的输入放行标签一并收掉，避免状态滞留。
	if (CombatWindowTag.IsValid())
	{
		ASC->RemoveLooseGameplayTag(CombatWindowTag);
	}
}
