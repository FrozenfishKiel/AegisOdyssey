#include "AegisOdyssey/UI/Widgets/Inventory/AOInventoryPageUI.h"

#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOHealthAttributeSet.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Equipment/AOQuickBarComponent.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentManagerComponent.h"
#include "AegisOdyssey/Equipment/Formal/AOFormalEquipmentSlotInventoryComponent.h"
#include "AegisOdyssey/Interaction/Session/AOContainerInteractionSessionModel.h"
#include "AegisOdyssey/Interaction/Session/AOInteractionSessionModel.h"
#include "AegisOdyssey/Inventory/AOBackPackComponent.h"
#include "AegisOdyssey/SkillSystem/Components/AOSkillComponent.h"
#include "AegisOdyssey/SkillSystem/Components/AOSkillSlotInventoryComponent.h"
#include "AegisOdyssey/UI/Common/Inventory/AOBackPackUI.h"
#include "AegisOdyssey/UI/Common/Inventory/AOQuickBarUI.h"
#include "AegisOdyssey/UI/Widgets/FormalEquipment/AOFormalEquipmentBarUI.h"
#include "AegisOdyssey/UI/Widgets/Skill/AOSkillBarUI.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOInventoryPageUI)

void UAOInventoryPageUI::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshInventoryPageContexts();
}

bool UAOInventoryPageUI::HasTargetInventoryDisplayContext() const
{
	return TargetInventoryDisplayContext.OwnerActor != nullptr
		|| TargetInventoryDisplayContext.BackPackComponent != nullptr
		|| TargetInventoryDisplayContext.QuickBarComponent != nullptr
		|| TargetInventoryDisplayContext.FormalEquipmentInventory != nullptr
		|| TargetInventoryDisplayContext.SkillComponent != nullptr;
}

FText UAOInventoryPageUI::GetTargetInventoryDisplayName() const
{
	if (const AActor* TargetActor = TargetInventoryDisplayContext.OwnerActor.Get())
	{
		return FText::FromString(TargetActor->GetName());
	}

	return FText::GetEmpty();
}

bool UAOInventoryPageUI::IsTargetInventoryOwnerDead() const
{
	return IsInventoryOwnerDead(TargetInventoryDisplayContext.OwnerActor.Get());
}

void UAOInventoryPageUI::RefreshInventoryPageContexts()
{
	// 自身侧上下文仍由页面本地打包；目标侧上下文只消费 session 已注册好的结果。
	SelfInventoryDisplayContext = BuildSelfInventoryDisplayContext();
	TargetInventoryDisplayContext = BuildTargetInventoryDisplayContext();
	ApplyInventoryPageContexts();
}

void UAOInventoryPageUI::ApplyInventoryPageContexts()
{
	if (SelfBackPackPanel != nullptr)
	{
		SelfBackPackPanel->SetDisplayContext(SelfInventoryDisplayContext);
	}

	if (SelfQuickBarPanel != nullptr)
	{
		SelfQuickBarPanel->SetDisplayContext(SelfInventoryDisplayContext);
	}

	if (SelfFormalEquipmentPanel != nullptr)
	{
		SelfFormalEquipmentPanel->SetDisplayContext(SelfInventoryDisplayContext);
	}

	if (SelfSkillPanel != nullptr)
	{
		SelfSkillPanel->SetDisplayContext(SelfInventoryDisplayContext);
	}

	if (TargetBackPackPanel != nullptr)
	{
		TargetBackPackPanel->SetDisplayContext(TargetInventoryDisplayContext);
	}

	if (TargetQuickBarPanel != nullptr)
	{
		TargetQuickBarPanel->SetDisplayContext(TargetInventoryDisplayContext);
	}

	if (TargetFormalEquipmentPanel != nullptr)
	{
		TargetFormalEquipmentPanel->SetDisplayContext(TargetInventoryDisplayContext);
	}

	if (TargetSkillPanel != nullptr)
	{
		TargetSkillPanel->SetDisplayContext(TargetInventoryDisplayContext);
	}
}

FAOInventoryDisplayContext UAOInventoryPageUI::BuildSelfInventoryDisplayContext() const
{
	FAOInventoryDisplayContext Context;

	const APawn* OwningPawn = GetOwningPlayerPawn();
	Context.OwnerActor = const_cast<APawn*>(OwningPawn);

	if (OwningPawn == nullptr)
	{
		return Context;
	}

	Context.BackPackComponent = OwningPawn->FindComponentByClass<UAOBackPackComponent>();
	Context.QuickBarComponent = OwningPawn->FindComponentByClass<UAOQuickBarComponent>();
	Context.FormalEquipmentInventory = OwningPawn->FindComponentByClass<UAOFormalEquipmentSlotInventoryComponent>();
	Context.FormalEquipmentManager = OwningPawn->FindComponentByClass<UAOFormalEquipmentManagerComponent>();
	Context.SkillComponent = OwningPawn->FindComponentByClass<UAOSkillComponent>();
	Context.SkillSlotInventory = OwningPawn->FindComponentByClass<UAOSkillSlotInventoryComponent>();
	return Context;
}

FAOInventoryDisplayContext UAOInventoryPageUI::BuildTargetInventoryDisplayContext() const
{
	FAOInventoryDisplayContext Context;

	const UAOContainerInteractionSessionModel* ContainerSessionModel = GetOwningContainerSessionModel();
	if (ContainerSessionModel == nullptr)
	{
		return Context;
	}

	// Target-side context must come from the formal session registration set.
	// The page no longer assembles target components on its own.
	ContainerSessionModel->PopulateTargetInventoryDisplayContext(Context);
	return Context;
}

bool UAOInventoryPageUI::IsInventoryOwnerDead(const AActor* InventoryOwnerActor) const
{
	if (const AAOCharacter* Character = Cast<AAOCharacter>(InventoryOwnerActor))
	{
		if (const UAbilitySystemComponent* AbilitySystemComponent = Character->GetAbilitySystemComponent())
		{
			return AbilitySystemComponent->GetNumericAttribute(UAOHealthAttributeSet::GetHealthAttribute()) <= KINDA_SMALL_NUMBER;
		}
	}

	return false;
}
