#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AOAnimStateData.generated.h"
class UStateTree;
class UAOAnimInstance;

USTRUCT(BlueprintType)
struct FAOAnimStateContainer
{
	GENERATED_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag Tag;  //标签，暂时不知道有什么用

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStateTree> PrimaryStateTree;  //核心状态树（因为需要频繁使用所以不建议使用软引用）

};
UCLASS(MinimalAPI , BlueprintType)
class UAOAnimStateData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	FAOAnimStateContainer GetStateContainer() const {return AnimStateContainer;}
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly , meta = (AllowPrivateAccess))
	FAOAnimStateContainer AnimStateContainer;
};
