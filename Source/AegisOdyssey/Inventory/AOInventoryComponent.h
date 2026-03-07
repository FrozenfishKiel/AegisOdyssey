// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryInterface.h"
#include "Components/ActorComponent.h"
#include "Components/PawnComponent.h"
#include "AegisOdyssey/AbilitySystem/AOAbilitySet.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "AOInventoryComponent.generated.h"


class UAOInventoryManagerComponent;
class UAOInventoryItemDefinition;
class UAOInventoryItemInstance;
class UAOInventoryComponent;
struct FAOInventoryList;
class UAOEquipmentInstance;
class UMVVM_InventoryMenu;
//Inventory Manager.

USTRUCT(BlueprintType)
struct FAOInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
	FAOInventoryEntry() {}
	FAOInventoryEntry(UAOInventoryComponent* InInventoryComponentPtr) : SlotOwnerComponent(InInventoryComponentPtr) {}

	bool operator == (const FAOInventoryEntry& other) const
	{
		if (Instance != other.Instance)
		{
			return false;
		}
		return true;
	}
	bool operator == (const UAOInventoryItemInstance* InInstance) const
	{
		if (SlotOwnerComponent == nullptr) return false;
		if (InInstance == nullptr) return false;
		if (Instance != InInstance) return false;
		return true;
	}
	bool operator != (const UAOInventoryItemInstance* InInstance) const
	{
		if (Instance != InInstance) return true;
		return true;
	}

	void operator=(const FAOInventoryEntry& other)
	{
		//防止自赋值 
		if (this!=&other)
		{
			Instance = other.Instance;
			StackCount = other.StackCount;
			GrantedHandles = other.GrantedHandles;
			//SlotComponent不需要被改变
		}
	}

public:
	//允许InventoryList和AOInventoryManagerComponent在外界访问（私有）
	friend FAOInventoryList;
	friend UAOInventoryComponent;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAOInventoryItemInstance> Instance = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 StackCount = 0;
	
	UPROPERTY()
	int32 LastObservedCount = 0;

	// Authority-only list of granted handles
	UPROPERTY(BlueprintReadOnly,NotReplicated)
	FAOAbilitySet_GrantedHandles GrantedHandles;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAOInventoryComponent> SlotOwnerComponent;
};

USTRUCT(BlueprintType)
struct FAOInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	FAOInventoryList() {}
	FAOInventoryList(UAOInventoryComponent* InInventoryItemDefinitionPtr) : OwnerComponent(InInventoryItemDefinitionPtr) {}
	~FAOInventoryList() {}
	
	TArray<UAOInventoryItemInstance* > GetAllItems() const;
	
private:
	friend UAOInventoryComponent;
public:

	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	
	//~End of FFastArraySerializer contract
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FAOInventoryEntry, FAOInventoryList>(Entries, DeltaParms, *this);
	}
	
	UPROPERTY()
	TArray<FAOInventoryEntry> Entries;
	UPROPERTY(NotReplicated)
	TObjectPtr<UAOInventoryComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FAOInventoryList> : public TStructOpsTypeTraitsBase2<FAOInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};


UCLASS(Abstract)
class AEGISODYSSEY_API UAOInventoryComponent : public UPawnComponent , public IInventoryInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAOInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	//基于Definition模板的AddEntry
	UAOInventoryItemInstance* AddEntry(TSubclassOf<UAOInventoryItemInstance> ItemClass,
	                                   TSubclassOf<UAOInventoryItemDefinition> ItemDefClass, int32& ItemStackCount,
	                                   bool& bCheck);
	//基于ItemInstance添加的AddEntry
	void AddEntry(UAOInventoryItemInstance* Instance, int32& InCount, bool& bCheck);
	int32 FindAvaliableSlot(UAOInventoryItemInstance* Instance , int32 Count) const;
	//初始化或刷新背包格子
	virtual void InitializeOrRefreshInventorySlots();
	virtual void InitializeParams();

	void RemoveEntry(UAOInventoryItemInstance* ItemInstance);
	
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure=false)
	inline TArray<UAOInventoryItemInstance*> GetAllItems() const;
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure=false)
	inline TArray<FAOInventoryEntry> GetAllLists() const;

	UFUNCTION(BlueprintCallable,BlueprintAuthorityOnly,Category=Inventory)
	virtual void AddItemInstance(UAOInventoryItemInstance* ItemInstance, int32& InCount, bool& bCheck);
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	virtual void AddItemDefinition(TSubclassOf<UAOInventoryItemInstance> ItemClass,TSubclassOf<UAOInventoryItemDefinition> ItemDefClass, int32& StackCount, bool& bCheck);
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly,Category=Inventory)
	void RemoveItemInstance(UAOInventoryItemInstance* ItemInstance);
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void RemoveItemInstanceFromIndex(const int32 TargetIndex);


	UFUNCTION(BlueprintCallable, Server, Reliable)
	void WhenItemExchange(UAOInventoryComponent* TargetItemContainer , const int32 SourceIndex , const int32 TargetIndex);

	FAOInventoryEntry FindInventoryEntryFromInstance(UAOInventoryItemInstance* ItemInstance) const;
	virtual UAOInventoryComponent* GetInventoryComponent()  override {return this;}
	virtual inline TArray<FAOInventoryEntry> GetInventoryContainer() const {return InventoryList.Entries;}
public:	
	//~UObject interface
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	//~End of UObject interface

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
public:
	virtual void Client_BroadCastInventoryRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize){}
	virtual void Client_BroadCastInventoryAdd(const TArrayView<int32> AddIndices, int32 FinalSize, const TArray<FAOInventoryEntry>& TargetList){}
	virtual void Client_BroadCastInventoryChange(const TArrayView<int32> ChangedIndices, int32 FinalSize){}
	virtual void BroadCastInventoryChange(){}
protected:
	UPROPERTY(Replicated)
	FAOInventoryList InventoryList;
	//物品栏的格子
	UPROPERTY(EditAnywhere,BlueprintReadOnly , Category = "InventoryConfig")
	int32 NumSlots = 0;
};