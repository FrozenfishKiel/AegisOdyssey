// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "TestReplicationPawnComponent.generated.h"

/**
 * 测试用的网络复制Pawn组件，用于模拟AOVMPawnComponent的网络复制行为
 * 这个组件包含两个网络复制属性，用于测试服务器到客户端的同步流程
 */
UCLASS(Blueprintable, Meta=(BlueprintSpawnableComponent))
class AEGISODYSSEY_API UTestReplicationPawnComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UTestReplicationPawnComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 网络复制属性1：测试数据1
	UPROPERTY(ReplicatedUsing = OnRep_TestData1)
	FString TestData1;

	// 网络复制属性2：测试数据2
	UPROPERTY(ReplicatedUsing = OnRep_TestData2)
	int32 TestData2;

	// 设置测试数据1的方法
	UFUNCTION(BlueprintCallable, Category = "TestReplication")
	void SetTestData1(const FString& NewData);

	// 设置测试数据2的方法
	UFUNCTION(BlueprintCallable, Category = "TestReplication")
	void SetTestData2(int32 NewData);

	// 获取当前角色是否在服务器上运行
	UFUNCTION(BlueprintPure, Category = "TestReplication")
	bool IsRunningOnServer() const;

	// 获取当前角色是否在客户端上运行
	UFUNCTION(BlueprintPure, Category = "TestReplication")
	bool IsRunningOnClient() const;

protected:
	// 重写网络复制属性注册
virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// 网络复制回调函数1
	UFUNCTION()
	void OnRep_TestData1();

	// 网络复制回调函数2
	UFUNCTION()
	void OnRep_TestData2();

	// 打印服务器日志（绿色）
	void PrintServerLog(const FString& Message) const;

	// 打印客户端日志（黄色）
	void PrintClientLog(const FString& Message) const;

	// 打印通用日志（白色）
	void PrintLog(const FString& Message) const;

	// 定时器相关方法
protected:
	// 重写BeginPlay方法
	virtual void BeginPlay() override;

	// 定时器回调函数
	void OnTestTimer();

private:
	// 定时器句柄
	FTimerHandle TestTimerHandle;
};

/**
 * 测试用的简单Actor，用于承载测试组件
 */
UCLASS()
class AEGISODYSSEY_API ATestReplicationActor : public AActor
{
	GENERATED_BODY()

public:
	ATestReplicationActor();

	// 测试组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TestReplication")
	TObjectPtr<UTestReplicationPawnComponent> TestComponent;

protected:
	virtual void BeginPlay() override;
};