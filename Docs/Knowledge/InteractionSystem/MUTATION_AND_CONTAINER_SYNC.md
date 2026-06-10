---
title: Interaction Mutation And Container Sync
tags:
  - knowledge
  - interaction-system
  - mutation
  - container
  - sync
aliases:
  - Interaction Mutation And Container Sync
  - 交互系统Mutation与容器同�?---

# 交互系统Mutation与容器同�?更新时间�?026-05-19  
适用范围：当前交互会话内“修改当前交互对象数据”的统一调度链，以及容器详细内容在多人场景下的当前同步结构�? 
不适用范围：所有未来交互对象类型的完整同步策略细节�?
## 1. 当前统一 Mutation 链是什�?
当前代码里已经正式存在：

- `FAOInteractableMutationRequest`
- `SubmitCurrentInteractableMutation(...)`
- `PendingCurrentInteractableMutations`
- `RequestAcquireCurrentInteractableOwner()`
- `FlushPendingCurrentInteractableMutationsIfReady()`

这说明历史笔记里的“权限等�?+ 挂起队列 + 权限到位后放行”已经不是纯设计，而是当前实现的一部分�?
## 2. 当前 Mutation 请求结构

定义位置�?
- `Source/AegisOdyssey/Interaction/AOInteractionSessionComponent.h`

当前 `FAOInteractableMutationRequest` 至少包含�?
- `DebugName`
- `ValidateAction`
- `ExecuteAction`

这代表当前统一调度层看到的不是“交换请求专用类型”或“删除请求专用类型”，而是�?
**一个通用的、可验证、可执行的“当前交互对�?Mutation 请求”�?*

## 3. 当前统一调度语义

`SubmitCurrentInteractableMutation(...)` 当前的真实语义是�?
### 3.1 如果没有当前会话或没有当前交互对�?
当前会直接尝试执行请求�? 
这意味着统一调度链只在“当前会话上下文下修改当前交互对象数据”时才发挥作用�?
### 3.2 如果调用方在服务�?
当前会先确保请求当前交互对象 owner，然后直接尝试执行�?
### 3.3 如果客户端已经拥有当前交互对�?mutation authority

当前会直接执行�?
### 3.4 如果客户端还没有 authority

当前会：

- 把请求压�?`PendingCurrentInteractableMutations`
- 调用 `RequestAcquireCurrentInteractableOwner()`
- 等待 owner 切换完成后再统一放行

这就是当前真实落地的统一等待链�?
## 4. 当前 authority 判断边界

`HasCurrentInteractableMutationAuthority()` 当前逻辑很明确：

- 服务器天然有�?- 客户端只有在 `CurrentInteractableActor->GetOwner() == SessionOwnerActor` 时才视为有权

也就是说，当前交互对�?owner 是这�?mutation waiting 链的关键门槛�?
## 5. 当前排队与放行如何发�?
当前会话组件每帧会调用：

- `FlushPendingCurrentInteractableMutationsIfReady()`

它当前只关心三件事：

1. 有没有待处理 mutation
2. 当前会话和交互对象是否还有效
3. 当前是否已经具备 mutation authority

一旦满足条件，它会�?
- 取出整个 `PendingCurrentInteractableMutations`
- 逐个�?`CanExecute()`
- 逐个执行 `Execute()`

这说明当前调度层只处理“什么时候能执行”，不处理“具体执行什么业务”�?
## 6. 当前 UI 已经在复用统一 Mutation �?
当前已确认：

- `UAOInventoryUI::RequestExchangeBetweenInventories(...)`
- `UAOInventoryUI::RequestUseInventoryItem(...)`

都会在需要通过当前交互会话修改当前交互对象数据时，构�?`FAOInteractableMutationRequest` 并提交给�?
- `SubmitCurrentInteractableMutation(...)`

这说明当�?UI 层已经开始遵守统一调度模式，而不是每个动作自己维护一套权限等待�?
## 7. 当前容器详细内容同步结构

当前容器详细内容同步可以拆成四段�?
1. 服务端对象真实数据保存在 `UAOContainerInventoryComponent`
2. 数据变更后，�?`AAOChest::RefreshObservers()` 主动刷新观察者会�?3. `UAOContainerInteractionSessionModel::RefreshObservedContainer()` 生成 `ObservedContainerSlots`
4. `UAOInteractionSessionComponent` �?`ContainerSlots` 作为 `OwnerOnly` 会话状态复制给客户�?
然后客户端再通过�?
- `RebuildClientSessionFromReplicatedState()`

恢复本地容器会话模型，并�?`AOContainerUI` 用观察快照重建格子�?
## 8. 当前“观察者”在代码里的真实意义

历史文档里反复强调“观察者同步”�? 
当前代码里，它已经具体化为：

- `AAOChest::ActiveObservers`
- `RegisterObserver(...)`
- `UnregisterObserver(...)`
- `RefreshObservers()`

但要注意当前真实实现边界�?
- 服务端确实维护观察者会话列�?- 观察者刷新发生在服务端对象变更后
- 客户端看到的不是直接共享容器组件，而是 owner-only 的会话快�?
所以当前“观察者”应理解为：

**谁在服务端被视为这个容器的有效会话观察者，以及谁应获得自己的会话快照更新�?*

## 9. 当前容器 UI 的消费模�?
当前容器 UI 链是�?
- `UAOContainerUI` 绑定玩家自己�?`InteractionSessionComponent`
- 当当前会话切�?`UAOContainerInteractionSessionModel` 时，重绑容器会话模型
- 监听 `GetOnContainerDataChanged()`
- �?`ObservedContainerSlots` 重建 `UAOContainerSlot`

这说明当�?UI 不是直接订阅 `ChestInventory` 真相组件，而是订阅�?
- 当前玩家自己持有的会话模�?
这条边界非常重要，因为它保证了：

- 会话是上下文
- UI 是会话消费�?- 对象真实数据仍在对象�?
## 10. 当前多人容器一致性是如何保证�?
当前一致性链路是�?
1. 服务端上�?`UAOContainerInventoryComponent` 是权威数�?2. 每次库存变更都会触发 `BroadCastInventoryChange(...)`
3. 容器 owner `AAOChest` 会刷新所有有效观察�?4. 每个观察者会话把新的观察快照同步到自己的会话组件
5. 每个客户端只基于自己收到的会话快照刷�?UI

因此 A 玩家拿走一个物品后�?
- 真相先在服务端容器库存里变化
- 所有有效观察者会话都会重新取快照
- 每个对应客户端会看到自己的容�?UI 更新

这就是当前多人共享可见容器的实际落地点�?## 11. ��ק��������׼�򲹳�

������Ҫ������������Ϊ���Ѿ�ʵ����������ק���߷������ⷴ�����⡣

��ǰͳһԼ�����£�

1. ����ק��ڲ㣬`DraggedInventory / DraggedSlotIndex` ��Զ��ʾ��������������һ�ࡱ��
2. ����ק��ڲ㣬`DropInventory / DropSlotIndex` ��Զ��ʾ����ǰ�����һ�ࡱ��
3. ��Ҫ����ק��ڲ������ `Source / Target` ȥ���ı����ϳ��ࡢ�ı������ࡣ
4. ��ͼ��Widget����ק���ء��Ҽ��˵��ָ���ק������ʱ��������Χ�ơ�������� / ���ࡱȥ���⡣

��������׼���Ŀ�Ĳ�����д�ײ��潻�����壬���Ƿ�ֹ UI ����ʱ�����ߺ���ӷ���
�͵�ǰʵ�ֶ���ʱ������ֱ�Ӱ���ӳ��� `UAOInventoryUI::RequestExchangeBetweenInventories(DraggedInventory, DraggedSlotIndex, DropInventory, DropSlotIndex)` �� `UAOInventoryComponent::ExecuteExchangeRequest(DraggedInventory, DraggedSlotIndex, DropInventory, DropSlotIndex)` ��һ�������
���� `Dragged*` ʼ���Ǳ�����࣬`Drop*` ʼ�������ࡣ

## 12. �ײ㽻��������ζ�Ӧ����׼��

ͳһ��潻��������Ȼ���������п������֮��Ľ������滻�������

��ǰӦ�������⣺

1. ���������ܲ��ܽ��ձ�������Ŀ��
2. �������ԭ��������Ŀ�����ʱ�����ԭ���ܲ��ܽ��ձ��������ľ���Ŀ��
3. ǰ��У��ͨ�����ɱ��������Ϊ RPC �������壬�����������ύ������ˡ�

���Ժ����Ų���ק����ʱ���ȿ� UI ����û�а� dragged/drop ����ӷ������� `ExecuteExchangeRequest(...)` ���ꡣ
��ǰʵ������������û�ж���������ʽװ��ר�÷ֲ棬��ʽװ���ۡ���ͨ�����ۡ����������ն�����ͬһ�׽�����ڣ�ֻ�Ǹ�����ڲ��������ǰ��У�鲻ͬ��
