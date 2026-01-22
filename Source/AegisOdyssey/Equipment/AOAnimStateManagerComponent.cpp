#include "AOAnimStateManagerComponent.h"

#include "AegisOdyssey/Animation/AOAnimInstance.h"
#include "AegisOdyssey/Character/AOAnimStateData.h"
#include "AegisOdyssey/Character/AOCharacter.h"
#include "AegisOdyssey/Character/AOExtPawnComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AOAnimStateManagerComponent)

UAOAnimStateManagerComponent::UAOAnimStateManagerComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	
}

const UAOAnimInstance* UAOAnimStateManagerComponent::GetCurrentStateAnimInstance() const
{
	const UAOExtPawnComponent* ExtPawnComponent = GetOwner()->GetComponentByClass<UAOExtPawnComponent>();
	check(ExtPawnComponent);
	const UAOPawnData* AOPawnData = ExtPawnComponent->GetPawnData<UAOPawnData>();
	check(AOPawnData);

	UAOAnimStateData* AnimStateData = AOPawnData->AnimStateData;
	if (AnimStateData)
	{
		UAbilitySystemComponent* SourceASC = ExtPawnComponent->GetAbilitySystemComponent();  //从Extpawn中获取ASC
		if (!SourceASC) return nullptr;

		FGameplayTagContainer OwnedTags;
		SourceASC->GetOwnedGameplayTags(OwnedTags);

		for (const FGameplayTag& Tags : OwnedTags)
		{
			FAOAnimStateContainer AnimStateContainer = AnimStateData->GetAnimStateContainer(Tags);  //匹配标签
			if (AnimStateContainer.UAOTargetStateToLinkAnimLayer)
			{
				int32 RandomIndex = FMath::RandRange(0,AnimStateContainer.AOAnimations.Num()-1);
				return AnimStateContainer.AOAnimations[RandomIndex];
			}
		}
	}
	return nullptr;
}

void UAOAnimStateManagerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	LinkPlayerAnimation();
}

void UAOAnimStateManagerComponent::LinkPlayerAnimation()
{
	const UAOExtPawnComponent* ExtPawnComponent = GetOwner()->GetComponentByClass<UAOExtPawnComponent>();
	check(ExtPawnComponent);
	const UAOPawnData* AOPawnData = ExtPawnComponent->GetPawnData<UAOPawnData>();
	check(AOPawnData);
	AAOCharacter* AOCharacter = Cast<AAOCharacter>(GetOwner());

	UAOAnimStateData* AnimStateData = AOPawnData->AnimStateData;
	if (AnimStateData)
	{
		UAbilitySystemComponent* SourceASC = ExtPawnComponent->GetAbilitySystemComponent();  //从Extpawn中获取ASC
		if (!SourceASC) return;

		FGameplayTagContainer OwnedTags;
		SourceASC->GetOwnedGameplayTags(OwnedTags);

		for (const FGameplayTag& Tags : OwnedTags)
		{
			FAOAnimStateContainer AnimStateContainer = AnimStateData->GetAnimStateContainer(Tags);  //匹配标签
			if (AnimStateContainer.UAOTargetStateToLinkAnimLayer)
			{
				AOCharacter->GetMesh()->LinkAnimClassLayers(AnimStateContainer.UAOTargetStateToLinkAnimLayer);
			}
		}
	}
}

