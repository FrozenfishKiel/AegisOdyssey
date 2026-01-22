// Fill out your copyright notice in the Description page of Project Settings.

#include "TestReplicationPawnComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "TimerManager.h"
#include "Engine/World.h"

// 定义日志分类
DEFINE_LOG_CATEGORY_STATIC(LogTestReplication, Log, All);

UTestReplicationPawnComponent::UTestReplicationPawnComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 设置组件可以网络复制
	SetIsReplicatedByDefault(true);
	
	// 初始化测试数据
	TestData1 = TEXT("初始数据1");
	TestData2 = 100;
	
	PrintLog(TEXT("UTestReplicationPawnComponent构造函数被调用"));
}

void UTestReplicationPawnComponent::SetTestData1(const FString& NewData)
{
	if (HasAuthority())
	{
		PrintServerLog(FString::Printf(TEXT("服务器设置TestData1: %s -> %s"), *TestData1, *NewData));
		
		// 只有在服务器上才能修改数据
		if (TestData1 != NewData)
		{
			TestData1 = NewData;
			
			// 标记属性为脏数据，需要网络复制
			MARK_PROPERTY_DIRTY_FROM_NAME(UTestReplicationPawnComponent, TestData1, this);
			
			PrintServerLog(FString::Printf(TEXT("服务器标记TestData1为脏数据: %s"), *TestData1));
		}
	}
	else
	{
		PrintClientLog(FString::Printf(TEXT("客户端尝试设置TestData1被拒绝: %s"), *NewData));
	}
}

void UTestReplicationPawnComponent::SetTestData2(int32 NewData)
{
	if (HasAuthority())
	{
		PrintServerLog(FString::Printf(TEXT("服务器设置TestData2: %d -> %d"), TestData2, NewData));
		
		// 只有在服务器上才能修改数据
		if (TestData2 != NewData)
		{
			TestData2 = NewData;
			
			// 标记属性为脏数据，需要网络复制
			MARK_PROPERTY_DIRTY_FROM_NAME(UTestReplicationPawnComponent, TestData2, this);
			
			PrintServerLog(FString::Printf(TEXT("服务器标记TestData2为脏数据: %d"), TestData2));
		}
	}
	else
	{
		PrintClientLog(FString::Printf(TEXT("客户端尝试设置TestData2被拒绝: %d"), NewData));
	}
}

bool UTestReplicationPawnComponent::IsRunningOnServer() const
{
	return HasAuthority();
}

bool UTestReplicationPawnComponent::IsRunningOnClient() const
{
	return !HasAuthority();
}

void UTestReplicationPawnComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 注册网络复制属性，使用DOREPLIFETIME_CONDITION_NOTIFY宏
	// 模拟AOVMPawnComponent中的网络复制配置
	DOREPLIFETIME_CONDITION_NOTIFY(UTestReplicationPawnComponent, TestData1, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTestReplicationPawnComponent, TestData2, COND_None, REPNOTIFY_Always);
	
	PrintLog(TEXT("GetLifetimeReplicatedProps被调用，注册了2个网络复制属性"));
}

void UTestReplicationPawnComponent::OnRep_TestData1()
{
	// 这个函数在客户端上被调用，当TestData1从服务器复制过来时
	PrintClientLog(FString::Printf(TEXT("客户端收到TestData1复制: %s"), *TestData1));
	
	// 这里可以添加业务逻辑，比如更新UI等
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
			FString::Printf(TEXT("客户端: TestData1已更新为: %s"), *TestData1));
	}
}

void UTestReplicationPawnComponent::OnRep_TestData2()
{
	// 这个函数在客户端上被调用，当TestData2从服务器复制过来时
	PrintClientLog(FString::Printf(TEXT("客户端收到TestData2复制: %d"), TestData2));
	
	// 这里可以添加业务逻辑，比如更新UI等
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
			FString::Printf(TEXT("客户端: TestData2已更新为: %d"), TestData2));
	}
}

void UTestReplicationPawnComponent::PrintServerLog(const FString& Message) const
{
	// 服务器日志使用绿色
	UE_LOG(LogTestReplication, Log, TEXT("\x1b[32m[服务器] %s\x1b[0m"), *Message);
	
	// 同时在屏幕上显示
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
			FString::Printf(TEXT("[服务器] %s"), *Message));
	}
}

void UTestReplicationPawnComponent::PrintClientLog(const FString& Message) const
{
	// 客户端日志使用黄色
	UE_LOG(LogTestReplication, Log, TEXT("\x1b[33m[客户端] %s\x1b[0m"), *Message);
	
	// 同时在屏幕上显示
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
			FString::Printf(TEXT("[客户端] %s"), *Message));
	}
}

void UTestReplicationPawnComponent::PrintLog(const FString& Message) const
{
	// 通用日志使用白色
	UE_LOG(LogTestReplication, Log, TEXT("\x1b[37m[通用] %s\x1b[0m"), *Message);
	
	// 同时在屏幕上显示
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, 
			FString::Printf(TEXT("[通用] %s"), *Message));
	}
}

// 测试Actor的实现
ATestReplicationActor::ATestReplicationActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// 设置Actor可以网络复制
	SetReplicates(true);
	
	// 创建测试组件
	TestComponent = CreateDefaultSubobject<UTestReplicationPawnComponent>(TEXT("TestComponent"));
}

void ATestReplicationActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		UE_LOG(LogTestReplication, Log, TEXT("\x1b[32m[服务器] ATestReplicationActor开始播放\x1b[0m"));
	}
	else
	{
		UE_LOG(LogTestReplication, Log, TEXT("\x1b[33m[客户端] ATestReplicationActor开始播放\x1b[0m"));
	}
}

// UTestReplicationPawnComponent的定时器相关实现
void UTestReplicationPawnComponent::BeginPlay()
{
	Super::BeginPlay();
	
	PrintLog(TEXT("UTestReplicationPawnComponent开始播放"));
	
	// 获取世界和定时器管理器
	UWorld* World = GetWorld();
	if (World)
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		
		// 设置10秒后执行的定时器
		TimerManager.SetTimer(TestTimerHandle, this, &UTestReplicationPawnComponent::OnTestTimer, 10.0f, false);
		
		PrintLog(TEXT("已设置10秒后执行的测试定时器"));
	}
}

void UTestReplicationPawnComponent::OnTestTimer()
{
	PrintLog(TEXT("===== 10秒定时器触发，开始自动测试 ====="));
	
	if (HasAuthority())
	{
		// 服务器端测试
		PrintServerLog(TEXT("服务器端开始自动测试"));
		
		// 测试1: 初始数据验证
		PrintServerLog(FString::Printf(TEXT("初始TestData1: %s"), *TestData1));
		PrintServerLog(FString::Printf(TEXT("初始TestData2: %d"), TestData2));
		
		// 测试2: 修改TestData1
		PrintServerLog(TEXT("开始修改TestData1"));
		SetTestData1(TEXT("服务器修改后的数据1"));
		
		// 测试3: 修改TestData2
		PrintServerLog(TEXT("开始修改TestData2"));
		SetTestData2(999);
		
		// 测试4: 再次修改TestData2
		PrintServerLog(TEXT("再次修改TestData2"));
		SetTestData2(888);
		
		// 测试5: 测试相同值不触发修改
		PrintServerLog(TEXT("测试相同值不触发修改"));
		SetTestData2(888);
		
		PrintServerLog(TEXT("服务器端自动测试完成"));
	}
	else
	{
		// 客户端测试
		PrintClientLog(TEXT("客户端开始自动测试"));
		
		// 测试客户端权限控制
		PrintClientLog(TEXT("客户端尝试修改TestData1"));
		SetTestData1(TEXT("客户端尝试修改的数据1"));
		
		// 测试客户端权限控制
		PrintClientLog(TEXT("客户端尝试修改TestData2"));
		SetTestData2(777);
		
		PrintClientLog(TEXT("客户端自动测试完成"));
	}
	
	PrintLog(TEXT("===== 自动测试完成 ====="));
}