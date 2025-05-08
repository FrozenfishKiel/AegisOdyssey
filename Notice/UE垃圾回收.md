# 虚幻垃圾回收机制简析

虚幻引擎中，GC机制会通过标记和清除机制来管理对象的内存释放，当GC判定某个对象为不可达对象的时候，会对其进行垃圾回收



什么是不可达对象？什么才是不可达对象？

### 2. **标记阶段（Marking）——确定不可达对象**



虚幻引擎标记可达/不可达对象的过程称为可达性分析：敌人在发射火箭的时候，火箭由UWorld管理，此时有一个雷达对象引用火箭对象，但是不是持有者，如果此时火箭对象被销毁，也就是失去与UWorld的联系，但雷达扔保持这对火箭的引用，也就是说火箭仍是可到达的内存，此时该火箭从逻辑角度来说被销毁清除了，但GC并不会回收它，除非程序员手动清除



当标记UPROPERTY的时候，会让GC将其标记为可达对象

![33ad35e7-783a-4630-8436-bcf72752f61c](file:///C:/Users/frozenfish/Pictures/Typedown/33ad35e7-783a-4630-8436-bcf72752f61c.png)





TObjectPtr的三种变体：



![6b4203b7-8113-4425-b0f9-a9b650b06258](file:///C:/Users/frozenfish/Pictures/Typedown/6b4203b7-8113-4425-b0f9-a9b650b06258.png)



如果TWeakObjetPtr只保持对火箭的弱引用，即使没有显式地清除引用内存也可以被回收，但在解引用前一定要确保其有效

![3e39340b-8fe9-46e1-b43e-a35fb3ba71d3](file:///C:/Users/frozenfish/Pictures/Typedown/3e39340b-8fe9-46e1-b43e-a35fb3ba71d3.png)



以下是代码案例：

#### 错误示例：强引用导致内存未释放



// 雷达类持有火箭弹的强引用
UCLASS()
class ARadar : public AActor {
    GENERATED_BODY()
public:
    UPROPERTY()
    AActor* TrackedRocket; // 强引用
};

// 火箭弹爆炸后调用Destroy()
void ARocket::Explode() {
    Destroy(); // 标记为PendingKill，但GC前仍存在
}



强引用GC会跳过当前内存回收，下一次GC才会回收





#### 正确实践：使用弱引用



// 雷达类使用弱引用
UCLASS()
class ARadar : public AActor {
    GENERATED_BODY()
public:
    TWeakObjectPtr<AActor> TrackedRocket; // 弱引用
};

// 火箭弹销毁后，弱引用自动失效
void ARadar::UpdateTracking() {
    if (TrackedRocket.IsValid()) { // 检查有效性
        // 安全操作
    }
}

**结果**：火箭弹被销毁后，`TrackedRocket`自动置为`nullptr`





### **解决方案**

1. **合理使用引用类型**
   
   * 非必要持有对象时，使用`TWeakObjectPtr`避免强引用[3][6]。
   
   * 显式置空引用：在对象销毁后手动将引用设为`nullptr`。

2. **触发GC的条件控制**  
   通过调整`Time Between Purging Pending Kill Objects`参数或手动调用`ForceGarbageCollection()`立即回收[1]。

所有未被标记的对象会被移动到“待销毁队列”（`PendingKill` 列表），随后异步释放其内存。





#### 场景1：对象何时成为垃圾？

// 创建一个Actor并立即失去引用
AActor* TransientActor = GetWorld()->SpawnActor<AActor>();
TransientActor = nullptr; // 没有UPROPERTY()持有引用





TSoftObjectPtr用于持有资源的弱引用，其保存资源的路径但是不加载，只有实际需要的时候才会加载，正常情况下如果使用其他指针，那么对象内存会一直占据电脑内存，导致资源浪费；其他对象在使用的时候，要保证<u>强引用</u>，不然会被GC回收



虚幻引擎会标记根集对象何其所包含的所有引用



#### **(1) 显式根集（Explicit Roots）**

* **通过 `AddToRoot()` 显式标记的对象**：  
  手动添加到引擎根集的对象，强制GC保留它们，即使没有其他引用。
  UMyObject* MyObject = NewObject<UMyObject>();
  MyObject->AddToRoot(); // 显式标记为根集成员



* **通过 `UPROPERTY()` 声明的成员变量**：  
  被 `UPROPERTY()` 修饰的成员变量引用的对象，其所有者（如Actor或Component）存活时，这些对象会被自动视为根集的延伸。
  UPROPERTY()
  AActor* MyActor; // MyActor的存活取决于其所有者（如持有它的Actor）



#### **(2) 隐式根集（Implicit Roots）**

* **活跃的 `UWorld` 及其子对象**：  
  当前世界的所有Actor、Component、Level等均属于根集。

* **游戏实例（GameInstance）和全局管理器**：  
  如 `UGameInstance`、`UEngine` 单例等始终存活的对象。

* **渲染线程、异步加载线程等持有的资源**：  
  确保跨线程引用的对象不会被意外回收。





### **2. 虚幻GC的根集枚举流程**

虚幻引擎的根集枚举过程发生在 **垃圾回收的标记阶段**，具体步骤如下：

1. **暂停主线程（Stop-The-World）**：  
   为确保一致性，GC运行时所有游戏逻辑线程会被暂时挂起。

2. **遍历显式根集**：  
   收集所有通过 `AddToRoot()` 或 `UPROPERTY()` 标记的对象引用。

3. **遍历隐式根集**：  
   扫描当前 `UWorld`、全局管理器、线程资源等，收集其直接引用的对象。

4. **构建根集列表**：  
   将上述所有直接引用汇总为根集列表，作为后续递归标记的起点。

* * *

### **3. 根集枚举后的标记阶段**

完成根集枚举后，GC进入递归标记阶段：

* **从根集出发**：  
  遍历根集中的每一个对象，递归标记其引用的所有子对象（如Actor的Component、Component引用的Mesh等）。

* **标记为存活**：  
  所有被访问到的对象会被标记为 `Keep`，避免被回收。

未被标记的对象会被加入待销毁列表（`PendingKill`），在后续的清扫阶段释放内存。
