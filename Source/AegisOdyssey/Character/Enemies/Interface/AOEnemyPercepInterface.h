#pragma once
#include "CoreMinimal.h"
#include "AOEnemyPercepInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)

class UAOEnemyPercepInterface : public UInterface
{
	GENERATED_BODY()
public:
	
};
class IAOEnemyPercepInterface  //AI感官接口
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AO|Perception")
	AActor* GetSenseResultActor() const;
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AO|Perception")
	void SetSenseResultActor(AActor* SenseResultActor);
};