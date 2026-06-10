---
title: Formal Equipment And Inventory Use
tags:
  - knowledge
  - inventory-equipment
  - formal-equipment
  - inventory-use
aliases:
  - Formal Equipment And Inventory Use
  - 正式装备栏与库存使用链说�?---

# 正式装备栏与库存使用链说�?
更新时间�?026-05-20  
适用范围：当前正式装备栏的装�?卸下链、库存右键使用链、统一获得物品通知链�? 
不适用范围：具体某件装备资产怎么配、美�?Widget 外观、所有武器案例细节�?
## 1. 当前库存里的“使用”不是一种语�?
当前 `TryUseItemAtSlot(...)` 只是统一入口，不是统一业务�?
真正分流发生在物品实例层�?
1. 默认 `UAOInventoryItemInstance` 走消耗品语义
2. `UAOEquipmentInstance` 走武�?装备管理语义
3. `UAOFormalEquipmentInstance` 走正式装备管理语�?
所以“右键使用”当前正确理解是�?
**库存先统一入口，再由实例类型决定真正业务�?*

## 2. 消耗品当前怎么使用

优先看：

- `Source/AegisOdyssey/Inventory/AOInventoryItemInstance.*`
- `Source/AegisOdyssey/Inventory/Fragments/AOFragment_Consumable.*`

当前链路是：

1. 菜单只在 `CanUseFromInventory(...)` 为真时显示“使用�?2. `TryUseItemAtSlot(...)` 在服务端调用 `TryUseFromInventory(...)`
3. 默认实例会读�?`AOFragment_Consumable`
4. 逐个对使用�?ASC 应用 `EffectsToApply`
5. 成功后返�?`ConsumeCount = 1`
6. 库存组件再统一扣减堆叠并广播槽位变�?
这条链当前已经是正式实现，不是历史方案�?
## 3. 正式装备当前怎么从库存进入角色身�?
优先看：

- `Source/AegisOdyssey/Equipment/Formal/AOFormalEquipmentInstance.*`
- `Source/AegisOdyssey/Equipment/Formal/AOFormalEquipmentManagerComponent.*`

当前“使用一件正式装备”的真实语义不是直接改角色变量，而是�?
1. 先确认这件库存实例是 `UAOFormalEquipmentInstance`
2. �?`FormalEquipmentDefinition` 解析它属于哪个唯一槽类�?3. 正式装备管理器把它路由到对应正式�?4. 最终仍然调用统一库存交换�?
也就是说�?
**正式装备到角色身�?= 正式装备实例进入正式装备槽库存投影�?*

## 4. 正式装备的运行时真相如何同步

优先看：

- `Source/AegisOdyssey/Equipment/Formal/AOFormalEquipmentSlotInventoryComponent.cpp`
- `Source/AegisOdyssey/Equipment/Formal/AOFormalEquipmentManagerComponent.cpp`

当前同步方向是：

1. 正式装备槽库存投影变�?2. `BroadCastInventoryChange(...)` 触发同步
3. `SyncFormalEquipmentRuntimeFromInventoryProjection()`
4. `SyncFormalEquipmentFromInventoryProjection(...)`
5. 正式装备管理器更新每个槽�?`EquippedInstance`
6. 为当前装备授予或回收 `AbilitySetsToGrant`

当前已经确认�?
- 真相同步由服务端执行
- 客户端主要负责观察投影和刷新 UI

## 5. 正式装备当前怎么授予属�?
优先看：

- `Source/AegisOdyssey/Equipment/Formal/AOFormalEquipmentDefinition.*`
- `Source/AegisOdyssey/Equipment/Formal/AOFormalEquipmentManagerComponent.cpp`

当前正式做法是：

1. `AOFragment_FormalEquipment` 只声明槽类型
2. `UAOFormalEquipmentDefinition` 仍是正式装备静态定�?3. 属性、能力、AttributeSet 统一挂在 `AbilitySetsToGrant`
4. 管理器在装备时循�?`GiveToAbilitySystem(...)`
5. 卸下或替换时用记录下来的 `GrantedHandles` 回收

所以正式装备当前不是“Fragment 手动 Apply GE”那条旧路�?
## 6. 正式装备当前怎么拖拽与替�?
优先看：

- `Source/AegisOdyssey/UI/Widgets/FormalEquipment/AOFormalEquipmentSlotUI.*`
- `Source/AegisOdyssey/Inventory/AOInventoryComponent.cpp`

当前拖拽路径是：

1. 背包或其他来源容器拖起真实来源槽
2. 正式�?UI �?`DragOver / Drop` 阶段先还原真实来源槽
3. 正式�?UI �?`CanAcceptDraggedItemForThisFormalSlot(...)`
4. 管理器统一判断是否合法
5. 合法后请求统一交换
6. 来源槽与正式槽发生标准库存交�?7. 正式装备管理器对投影变化做同步响�?
这里有一条很重要的边界：

1. 正式�?UI 可以做前置拦截，但它不自己定义“头盔能不能进这个槽”的业务规则
2. 真正的合法性仍然收口在 `CanAcceptSourceItemForFormalSlot(...)`
3. 即使 UI 拖放入口已经做了拦截，底层统一交换链也还会再经�?`CanAcceptInventoryEntryAtSlot(...)` 这一层校�?
当前替换也复用这条链，因此旧装备不是被“销毁重建”，而是被交换回去�?
�͵�ǰʵ�ֶ���ʱ�����ﲻҪ�ѡ���ʽװ���ۡ�����ɵײ㽻��������� `DraggedInventory`���� `AOFormalEquipmentSlotUI::RequestEquipDraggedSourceSlotToThisFormalSlot(...)` �`InSourceContainer/InSourceSlotIndex` ���Ǳ�����࣬`SourceContainer/ObservedSlotIndex` �������࣬Ȼ��ͳһ���䵽 `RequestExchangeBetweenInventories(DraggedInventory, InSourceSlotIndex, DropInventory, DropSlotIndex)`��
�ײ�У����Ȼ��ȫ���� `UAOInventoryComponent::ExecuteExchangeRequest(...)`��������ʽװ��ֻ�ǻ���һ��������ƣ�û�ж����Ľ�������ֲ档
## 7. 正式装备当前怎么卸下

优先看：

- `Source/AegisOdyssey/UI/Widgets/FormalEquipment/AOFormalEquipmentSlotUI.cpp`
- `Source/AegisOdyssey/Equipment/Formal/AOFormalEquipmentManagerComponent.cpp`

当前正式槽右键菜单已经不是普通库�?Use 菜单，而是优先暴露�?
- `卸下`

当前卸下链路是：

1. 单槽 UI 构造右键菜�?2. 选择“卸下”后调用 `RequestUnequipFormalSlot(...)`
3. 管理器找到正式装备槽里的实例
4. 把它交换�?`BackPack`
5. 槽位投影变化后回收对�?`GrantedHandles`

这条链当前明确依赖背包有空位�?
## 8. 正式装备栏当前如何进 UI

优先看：

- `Source/AegisOdyssey/UI/ViewModel/Inventory/MVVM_InventoryMenu.*`
- `Source/AegisOdyssey/UI/Widgets/FormalEquipment/AOFormalEquipmentBarUI.*`
- `Source/AegisOdyssey/UI/Widgets/FormalEquipment/AOFormalEquipmentSlotUI.*`

当前正式装备�?UI 已经走：

1. `FormalEquipmentList`
2. `OnFormalEquipmentListChangedDynamic`
3. 整条装备栏重建槽 Widget
4. 每个槽消费自己的 `FAOInventoryEntry` 快照
5. 每个槽还会拿到自己的槽索引、槽类型和槽显示�?
这意味着正式装备栏当前已经有独立 ViewModel 字段，不需要塞回背包列表或快捷栏列表里混用�?
这层现在可以分成两部分来理解�?
1. `FormalEquipmentList` 负责“这一帧五个正式槽里分别装了什么�?2. 正式装备管理器负责“这五个槽本身分别是谁�?
所以正式装备栏当前不是一排普通物品格子，而是五个固定语义槽位上的物品快照�?
## 9. 获得物品通知当前怎么�?
优先看：

- `Source/AegisOdyssey/Inventory/AOInventoryComponent.*`
- `Source/AegisOdyssey/Inventory/AOInventoryMessageSubsystem.*`
- `Source/AegisOdyssey/UI/AOHUDViewModelComponent.cpp`
- `Source/AegisOdyssey/UI/ViewModel/MVVM_HUD.*`
- `Source/AegisOdyssey/UI/Widgets/HUD/AOMainUI.cpp`

当前正式链路是：

1. 成功入包
2. 背包组件构建 `FAOInventoryAcquisitionMessage`
3. 世界子系统广�?4. HUD 桥接层只保留本地玩家相关通知
5. `UMVVM_HUD` 维护最新一条和待消费队�?6. `AOMainUI` 或蓝图提示层消费这批通知

这条链当前只对“真正进入背包”的结果生效�?
## 10. 本轮确认的边�?
1. 正式装备栏和武器快捷栏共享库存实例语义，但不共享真相层�?2. 正式装备栏已进入 `投影 -> ViewModel -> UI` 的观察结构�?3. 消耗品使用、正式装备使用、武器装备切槽都在复用同一个“库存使用入口”，但后续业务分流完全不同�?4. 获取物品通知当前已经是正式链路，不应再在来源玩法里手写第二套 HUD 提示�?
## 11. ��ʽװ����ק��ڵ��������岹��

������ⲹһ�������ױ����ӵ�ʵ��׼��

1. ����ʽװ����ק��ڲ㣬ǰһ����Զ����������ࡱ���⡣
2. ��ǰ��ʽװ�����Լ���Զ������ǰ���ࡱ���⡣
3. ��Ҫ�ѡ���ʽװ��������β�����Ŀ�ꡱ��д�ɵײ������ġ�������ࡱ��

�������֣�

- �ϱ�������ʽװ��ʱ��UI ��Ϊ�Լ��ڴ���㣬����ȴ�ѱ�������㡣
- �����ӵ�����ʱ��UI ��Ϊ�Լ��ڴ�Ŀ�꣬����ȴ�����ӵ��ɱ�����ࡣ

�������Ȿ�ʲ�����ʽװ��ר�����⣬������ק���������������ʧ�浼�µ�ͳһ���ߴ���
��ǰ������������������λ�þ��� `DraggedInventory / DraggedSlotIndex` �� `DropInventory / DropSlotIndex` �����֣����ն����䵽ͬһ����潻�������ϣ������ڡ���ʽװ������һ�׽��� RPC����ʵ�֡�
