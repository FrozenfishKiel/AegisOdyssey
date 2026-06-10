#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "AegisOdyssey/AbilitySystem/AOAbilitySystem.h"
#include "AegisOdyssey/AbilitySystem/Attributes/Core/AOHealthAttributeSet.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Character/Enemies/AOEnemy.h"
#include "AegisOdyssey/Interaction/InteractableTarget.h"
#include "AegisOdyssey/Inventory/InventoryInterface.h"
#include "Components/BoxComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCharacterInventoryInteractionImplementsFormalInterfacesTest,
	"AegisOdyssey.Interaction.CharacterInventory.ImplementsFormalInterfaces",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCharacterInventoryInteractionReturnsBackPackInventoryTest,
	"AegisOdyssey.Interaction.CharacterInventory.ReturnsBackPackInventory",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCharacterInventoryInteractionHasInteractionBoundsTest,
	"AegisOdyssey.Interaction.CharacterInventory.HasInteractionBounds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAliveEnemyCharacterInventoryInteractionDeniedTest,
	"AegisOdyssey.Interaction.CharacterInventory.AliveEnemyDenied",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDeadEnemyCharacterInventoryInteractionAllowedTest,
	"AegisOdyssey.Interaction.CharacterInventory.DeadEnemyAllowed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

namespace
{
static UAOHealthAttributeSet* InitializeTestHealthAttributes(AAOCharacter* Character)
{
	if (Character == nullptr)
	{
		return nullptr;
	}

	UAOAbilitySystem* AbilitySystem = Cast<UAOAbilitySystem>(Character->GetAbilitySystemComponent());
	if (AbilitySystem == nullptr)
	{
		return nullptr;
	}

	return Cast<UAOHealthAttributeSet>(AbilitySystem->EnsureSpawnedAttributeSet(UAOHealthAttributeSet::StaticClass()));
}
}

bool FCharacterInventoryInteractionImplementsFormalInterfacesTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	TestTrue(
		TEXT("AAOCharacter should implement IInteractableTarget."),
		AAOCharacter::StaticClass()->ImplementsInterface(UInteractableTarget::StaticClass()));

	TestTrue(
		TEXT("AAOCharacter should implement IInventoryInterface."),
		AAOCharacter::StaticClass()->ImplementsInterface(UInventoryInterface::StaticClass()));

	return true;
}

bool FCharacterInventoryInteractionReturnsBackPackInventoryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	AAOCharacter* TestCharacter = NewObject<AAOCharacter>(GetTransientPackage());
	TestNotNull(TEXT("Should create character test object."), TestCharacter);
	if (TestCharacter == nullptr)
	{
		return false;
	}

	IInventoryInterface* InventoryInterface = Cast<IInventoryInterface>(TestCharacter);
	TestNotNull(TEXT("Character should expose inventory interface."), InventoryInterface);
	if (InventoryInterface == nullptr)
	{
		return false;
	}

	TestNotNull(
		TEXT("Character inventory interaction should resolve to the backpack component."),
		InventoryInterface->GetInventoryComponent());

	return true;
}

bool FCharacterInventoryInteractionHasInteractionBoundsTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	AAOCharacter* TestCharacter = NewObject<AAOCharacter>(GetTransientPackage());
	TestNotNull(TEXT("Should create character interaction-bounds test object."), TestCharacter);
	if (TestCharacter == nullptr)
	{
		return false;
	}

	UBoxComponent* InteractionBounds = TestCharacter->FindComponentByClass<UBoxComponent>();
	TestNotNull(TEXT("Character should own an interaction bounds box."), InteractionBounds);
	return true;
}

bool FAliveEnemyCharacterInventoryInteractionDeniedTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	AAOEnemy* AliveEnemy = NewObject<AAOEnemy>(GetTransientPackage());
	TestNotNull(TEXT("Should create alive enemy test object."), AliveEnemy);
	if (AliveEnemy == nullptr)
	{
		return false;
	}

	UAOHealthAttributeSet* HealthSet = InitializeTestHealthAttributes(AliveEnemy);
	TestNotNull(TEXT("Enemy should expose health attributes."), HealthSet);
	if (HealthSet == nullptr)
	{
		return false;
	}

	HealthSet->SetMaxHealth(100.0f);
	HealthSet->SetHealth(100.0f);

	TestFalse(
		TEXT("Alive enemies should not expose formal inventory-container access."),
		AliveEnemy->CanOpenInventoryAsContainer());

	return true;
}

bool FDeadEnemyCharacterInventoryInteractionAllowedTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	AAOEnemy* DeadEnemy = NewObject<AAOEnemy>(GetTransientPackage());
	TestNotNull(TEXT("Should create dead enemy test object."), DeadEnemy);
	if (DeadEnemy == nullptr)
	{
		return false;
	}

	UAOHealthAttributeSet* HealthSet = InitializeTestHealthAttributes(DeadEnemy);
	TestNotNull(TEXT("Enemy should expose health attributes."), HealthSet);
	if (HealthSet == nullptr)
	{
		return false;
	}

	HealthSet->SetMaxHealth(100.0f);
	HealthSet->SetHealth(0.0f);

	TestTrue(
		TEXT("Dead characters should allow inventory-container access before actor destruction."),
		DeadEnemy->CanOpenInventoryAsContainer());

	return true;
}

#endif
