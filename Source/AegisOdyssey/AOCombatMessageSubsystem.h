#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AOCombatResultMessage.h"
#include "AOCombatMessageSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FAOCombatResultMessageNativeDelegate, const FAOCombatResultMessage&);

// 世界级战斗结果消息子系统。
// 它只负责把已经完成结算的战斗真相在本地世界里广播；
// 当运行在服务端时，还会把相关结果定向转发给相关客户端，再由客户端在本地世界里复播。
UCLASS()
class AEGISODYSSEY_API UAOCombatMessageSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 世界级统一战斗消息入口。
	// 这里只暴露已经完成结算的正式结果，不解释任何战斗真相。
	UFUNCTION(BlueprintPure, Category = "AO|Combat", meta = (WorldContext = "WorldContextObject"))
	static UAOCombatMessageSubsystem* Get(const UObject* WorldContextObject);

	// 对外广播一条正式战斗结果。
	// 所有只读订阅方都应把这里视为战斗系统的真相输出口。
	void BroadcastCombatResult(const FAOCombatResultMessage& Message);
	void BroadcastCombatResultLocal(const FAOCombatResultMessage& Message);

	FAOCombatResultMessageNativeDelegate OnCombatResultMessage;
};
