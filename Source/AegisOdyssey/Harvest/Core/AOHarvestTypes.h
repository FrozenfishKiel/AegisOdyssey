#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "AOHarvestTypes.generated.h"

class UAOHarvestToolDefinition;
class UAOHarvestToolFragment;
class UAOHarvestToolInstance;
class UAOHarvestToolProfile;
class UAOHarvestableComponent;
class UAnimMontage;

UENUM(BlueprintType)
enum class EAOHarvestRewardTiming : uint8
{
	PerHit UMETA(DisplayName = "Per Hit"),
	OnDepleted UMETA(DisplayName = "On Depleted"),
	Both UMETA(DisplayName = "Both")
};

USTRUCT(BlueprintType)
struct FAOHarvestRespawnConfig
{
	GENERATED_BODY()

	// 这里只描述节点静态上“会不会重生”。
	// 具体何时进入 depleted、何时真正启动局内再生定时器，属于运行时组件职责。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest", meta = (InlineEditConditionToggle))
	bool bCanRespawn = false;

	// 这里只表示“节点采空后，当前会话里经过多久再恢复”。
	// 当前明确只服务采集节点的局内再生，不承担种植、熔炉、烹饪那类跨会话时间推进语义。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest", meta = (EditCondition = "bCanRespawn", ClampMin = "0.0", UIMin = "0.0"))
	float RespawnInterval = 0.0f;
};

USTRUCT(BlueprintType)
struct FAOHarvestHitCheckConfig
{
	GENERATED_BODY()

	// 这是工具定义层给出的命中校验参数模板。
	// 第四阶段的服务器最终判定会基于这些参数执行距离、Sweep、朝向和遮挡检测。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MaxDistance = 220.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float SweepRadius = 24.0f;

	// Harvest hit traces must come from the equipped tool itself, not from the camera.
	// Designers author these two sockets on the spawned tool mesh to define the real swing segment.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	FName TraceStartSocketName = TEXT("HarvestStart");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	FName TraceEndSocketName = TEXT("HarvestEnd");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest", meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"))
	float MaxFacingAngleDegrees = 55.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	bool bCheckOcclusion = true;
};

USTRUCT(BlueprintType)
struct FAOHarvestDropEntry
{
	GENERATED_BODY()

	// 第一阶段先统一用数字 ItemId 表达掉落身份。
	// 真正的 ItemId -> Definition / Instance 查询，后续由物品总表 DataTable 负责。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	int32 ItemId = INDEX_NONE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float BaseDropChance = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest", meta = (ClampMin = "0", UIMin = "0"))
	int32 MinCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest", meta = (ClampMin = "0", UIMin = "0"))
	int32 MaxCount = 1;

	// 奖励时机已经拍板为“按掉落条目配置”，而不是系统全局写死一套。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	EAOHarvestRewardTiming RewardTiming = EAOHarvestRewardTiming::PerHit;
};

USTRUCT(BlueprintType)
struct FAOHarvestToolTuning
{
	GENERATED_BODY()

	// 这是节点面对某种工具机械语义时给出的响应结果。
	// 它表达的是“这个对象如何看待这类工具”，而不是工具自己宣称能采什么。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	bool bCanHarvest = true;

	// 这次命中对“节点进度扣减量”的倍率修正。
	// 它会乘到工具自己的 BaseHarvestPower 上，决定这一下到底能推进多少采集进度；它不是掉落数量倍率。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ProgressMultiplier = 1.0f;

	// 这次命中对“最终奖励数量”的倍率修正。
	// 它只影响掉落条目结算出来的 Count，不影响节点进度推进；因此它和 ProgressMultiplier 是两条独立语义。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float YieldMultiplier = 1.0f;

	// 稀有掉落概率附加值。
	// 当前直接叠加到条目的 BaseDropChance 上，再统一 Clamp 到 [0, 1]。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	float RareDropChanceBonus = 0.0f;
};

USTRUCT(BlueprintType)
struct FAOHarvestToolProfileResponse
{
	GENERATED_BODY()

	// 这里用可扩展 Profile 资产做键，而不是把工具机械语义写死成 C++ 枚举。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	TObjectPtr<UAOHarvestToolProfile> ToolProfile = nullptr;

	// 当前节点面对这个 ToolProfile 时采用的完整响应配置。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Harvest")
	FAOHarvestToolTuning Response;
};

USTRUCT(BlueprintType)
struct FAOHarvestNodeRuntimeState
{
	GENERATED_BODY()

	// 第二阶段开始，这里专门承载“节点自己当前跑到了哪”这类运行时状态。
	// 它只属于节点组件，不属于角色，也不属于工具。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harvest")
	float CurrentProgress = 0.0f;

	// depleted 只表达“当前不可再采”，不区分它是永久空、还是正在等待本局内恢复。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harvest")
	bool bDepleted = false;

	// 标记节点当前是否已经进入“等待局内再生”的正式运行时状态。
	// 这样程序员和客户端观察方都能区分“已经采空但会恢复”和“就是不会恢复”。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harvest")
	bool bRespawnPending = false;
};

USTRUCT(BlueprintType)
struct FAOHarvestRuntimeContext
{
	GENERATED_BODY()

	// ToolDefinition / ToolInstance 是这次挥击开始时采集工具的最小稳定快照。
	// 激活数据过网时只保留服务端可重建的工具身份，不直接传 Fragment 这类定义内嵌子对象。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	TObjectPtr<const UAOHarvestToolDefinition> ToolDefinition = nullptr;

	// 如果当前工具已经具备正式实例层，运行时主链优先保留这一份实例快照。
	// 这样耐久、词条、临时加成等实例级语义后续就能顺着这条链扩展。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	TObjectPtr<const UAOHarvestToolInstance> ToolInstance = nullptr;

	// ToolFragment 只在当前进程内作为便捷缓存使用。
	// 真正联机时由服务端根据 ToolInstance / ToolDefinition 现场重建，不把它当作稳定网络字段。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	TObjectPtr<const UAOHarvestToolFragment> ToolFragment = nullptr;

	// TargetActor / TargetComponent 只在命中窗口里真实打到一个可采对象后才会被解析出来。
	// 它们不参与激活数据过网，避免把“命中后临时上下文”误当成“激活前稳定快照”。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	TObjectPtr<UAOHarvestableComponent> TargetComponent = nullptr;
};

USTRUCT(BlueprintType)
struct FAOHarvestHitContext
{
	GENERATED_BODY()

	// HitContext 是“提交一次采集命中请求”时需要带上的最小上下文。
	// 它表达的是“这次挥击从哪里发起、朝哪里打过去、用的是什么工具、最后真实命中了谁”。
	// 服务端最终只基于这一次挥击的真实命中快照做重判定与统一结算。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	FAOHarvestRuntimeContext RuntimeContext;

	// 这里显式记录发起采集的执行者。
	// 我们不在角色身上挂常驻采集组件，所以第四阶段服务端重判定需要直接拿到这次请求的采集者。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	TObjectPtr<AActor> HarvesterActor = nullptr;

	// 这些是提交到服务端做最终重判定的命中参数快照。
	// 这里记录的是“本次请求声称自己从哪里发起、朝哪里打过去”，不是节点自己的静态配置。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	FVector TraceStart = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	FVector TraceEnd = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	FVector FacingDirection = FVector::ForwardVector;

	// 这里记录的是“本次工具挥击真正命中到目标时”的命中空间快照。
	// 后续如果树木、矿石之类的耗尽反应需要倒地方向、特效落点之类的信息，就直接从这里拿。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	FVector HitNormal = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	bool bHasHitData = false;
};

USTRUCT(BlueprintType)
struct FAOHarvestRewardEntry
{
	GENERATED_BODY()

	// 奖励条目先继续只保存统一的物品数字 ID。
	// 真正的 ID -> 物品定义查询仍然交给全局物品 DataTable 与后续库存链路。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harvest")
	int32 ItemId = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harvest")
	int32 Count = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harvest")
	EAOHarvestRewardTiming RewardTiming = EAOHarvestRewardTiming::PerHit;
};

USTRUCT(BlueprintType)
struct FAOHarvestResult
{
	GENERATED_BODY()

	// 第四阶段的统一结算结果先只回答“这次服务器最终提交是否成立”。
	// 第五阶段再基于它把奖励统一导入正式入包链路。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harvest")
	bool bSuccess = false;

	// 当前先保留轻量 RejectReason，方便日志定位与后续 HUD/漂字扩展。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harvest")
	FName RejectReason = NAME_None;

	// RequestedProgress 是这次请求理论上想扣掉多少。
	// AppliedProgress 才是结合节点当前剩余进度后，服务端最终真正扣掉多少。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harvest")
	float RequestedProgress = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harvest")
	float AppliedProgress = 0.0f;

	// Previous / Remaining 用来明确描述“扣减前”和“扣减后”的节点状态快照，
	// 方便日志、联机观察和后续 HUD 直接消费，而不是让上层自己反推。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harvest")
	float PreviousProgress = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harvest")
	float RemainingProgress = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harvest")
	bool bDepletedAfterHit = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Harvest")
	TArray<FAOHarvestRewardEntry> RewardEntries;
};

USTRUCT(BlueprintType)
struct FAOHarvestLifecycleContext
{
	GENERATED_BODY()

	// 统一的“节点耗尽生命周期”上下文。
	// 如果这次耗尽来自服务端真实命中，这里会带上采集者和命中空间数据；
	// 如果只是客户端收到复制后的兜底回放，这些字段可能为空。
	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	FAOHarvestResult HarvestResult;

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	TObjectPtr<AActor> HarvesterActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	FVector HarvesterForward = FVector::ForwardVector;

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	bool bHasHarvesterForward = false;

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	FVector HitNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	FVector HitDirection = FVector::ForwardVector;

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	bool bHasHitData = false;
};

USTRUCT(BlueprintType)
struct FAOHarvestReplicatedDepletedEvent
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Sequence = 0;

	UPROPERTY()
	float PreviousProgress = 0.0f;

	UPROPERTY()
	float RemainingProgress = 0.0f;

	UPROPERTY()
	float AppliedProgress = 0.0f;

	UPROPERTY()
	bool bSuccess = false;

	UPROPERTY()
	bool bDepletedAfterHit = false;

	UPROPERTY()
	TObjectPtr<AActor> HarvesterActor = nullptr;

	UPROPERTY()
	FVector HarvesterForward = FVector::ForwardVector;

	UPROPERTY()
	bool bHasHarvesterForward = false;

	UPROPERTY()
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector HitNormal = FVector::UpVector;

	UPROPERTY()
	FVector HitDirection = FVector::ForwardVector;

	UPROPERTY()
	bool bHasHitData = false;
};

USTRUCT(BlueprintType)
struct FAOHarvestTargetData : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	FAOHarvestTargetData()
		: InputTag(FGameplayTag::EmptyTag)
		, PlayRate(1.0f)
		, StartSection(NAME_None)
		, StartTime(0.0f)
	{
	}

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	FGameplayTag InputTag;

	// 这次采集动作具体播哪段蒙太奇，由状态树任务在激活时下发。
	// 这样采集和战斗保持同一套思路：StateTree 负责动作配置，Ability 负责消费与执行。
	// 注意：这里下发的是动作配置和工具快照，不是“已经选好的采集目标”。
	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	TSoftObjectPtr<UAnimMontage> Montage;

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	float PlayRate;

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	FName StartSection;

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	float StartTime;

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	FAOHarvestRuntimeContext RuntimeContext;

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FAOHarvestTargetData::StaticStruct();
	}

	virtual FString ToString() const override
	{
		const FString TargetName = RuntimeContext.TargetActor ? RuntimeContext.TargetActor->GetName() : TEXT("None");
		return FString::Printf(TEXT("FAOHarvestTargetData: Target=%s, Montage=%s, PlayRate=%.2f"),
			*TargetName,
			*Montage.ToString(),
			PlayRate);
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << InputTag;
		Ar << Montage;
		Ar << PlayRate;
		Ar << StartSection;
		Ar << StartTime;
		Ar << RuntimeContext.ToolDefinition;
		Ar << RuntimeContext.ToolInstance;
		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FAOHarvestTargetData> : public TStructOpsTypeTraitsBase2<FAOHarvestTargetData>
{
	enum
	{
		WithNetSerializer = true,
	};
};
