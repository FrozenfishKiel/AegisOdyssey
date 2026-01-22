#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AOAnimStateData.generated.h"
class UAOAnimInstance;

USTRUCT(BlueprintType)
struct FAOAnimStateContainer
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly)
	TArray<const UAOAnimInstance*> AOAnimations; //一个标签对应一个角色动画，但是我想对应多个

	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly)
	TSubclassOf<UAOAnimInstance> UAOTargetStateToLinkAnimLayer;  //选择角色要链接的Layer
};
UCLASS(MinimalAPI , BlueprintType)
class UAOAnimStateData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	const FAOAnimStateContainer& GetAnimStateContainer(const FGameplayTag TargetTag);
private:
	UPROPERTY(EditDefaultsOnly , meta = (AllowPrivateAccess))
	TMap<FGameplayTag,FAOAnimStateContainer> AnimStates;
};
