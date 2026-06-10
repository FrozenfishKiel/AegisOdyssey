# GAS 里 MMC 与动态 AttributeSet 的联动问题复盘

这次排查其实很有代表性，因为它表面上看像是“MMC 没生效”，但往下拆之后会发现，问题没有这么简单。

最开始看到的现象是这样的：武器和正式装备在装备、卸下之后，来源属性本身已经变化了，比如 `WeaponAttack`、`EquipmentDefense` 这些值都在变，但角色身上的最终属性，比如 `Attack`、`Defense`、`CritChance`、`Resistance` 却没有跟着变化。再往前一步看，就会自然怀疑是不是 `MMC` 写法有问题，或者 `Infinite GE` 对这种动态来源根本不支持。

这一轮排查之后，结论不是一句“支持”或者“不支持”就能说清的。真正值得记住的，是要把这几个概念拆开看：`MMC` 负责怎么算，`GameplayEffect` 负责把计算结果落到属性上，`AttributeSet` 提供属性来源，而动态注册的 `AttributeSet` 又会给整条链路带来额外的边界条件。

## 先把这次问题的现象钉住

当前项目里的链路大致是这样的。

角色最终参与战斗和 HUD 展示的那批属性，还在 `UAOCombatAttributeSet` 上。比如：

- `Attack`
- `Defense`
- `CritChance`
- `CritDamage`
- `Resistance`

而这次装备系统改造之后，新拆出来的“来源属性”放到了动态属性集里：

- `UAOWeaponAttributeSet`
- `UAOEquipmentAttributeSet`

也就是说，角色最后真正用来结算的还是“最终属性”，但最终属性的计算依赖，已经改成了“角色本体属性 + 武器来源属性 + 装备来源属性”。

举个最直接的例子，现在攻击力的语义已经不是写死的单值了，而是：

`Attack = Strength + WeaponAttack + EquipmentAttack`

这正是 `MMC_CalculateAttack` 在做的事。

所以从设计方向上看，这条思路本身没有问题。问题出在，为什么来源属性已经变了，最终属性却没动。

## 这次最容易误判的地方

最容易犯的错误，就是把下面两件事混成一件事：

1. `MMC` 能不能捕获别的属性来参与计算  
2. `MMC` 挂在一个持续 GE 上之后，能不能随着依赖变化自动刷新

第一件事答案很明确，能，而且这就是它存在的意义。

如果 `MMC_CalculateAttack` 不能捕获 `Strength`、`WeaponAttack`、`EquipmentAttack`，那它根本就没有必要存在，直接写死数值就行了。既然我们专门写了这些 `MMC`，说明项目本来就希望把最终属性建成“派生属性”，而不是每次靠手写逻辑重算。

第二件事就不能一句话下结论了。因为这里牵扯到的已经不是“会不会算”，而是“谁来触发这次重算”。

## 先说清楚 MMC 在 GAS 里的真实职责

`MMC` 不是一个全局自动监听器，它不会因为你项目里存在这个类，就自己到处接线。

它真正做的事情是：

- 某个 `GameplayEffect` 的某个 `Modifier` 需要一个数值
- 这个数值不想写死，也不想只靠 `ScalableFloat`
- 那就把这个数值的求法交给 `MMC`

所以 `MMC` 本质上是“某个 Modifier 的计算器”，而不是“一个脱离 GE 独立工作的系统”。

这句话很重要，因为它直接决定了我们排查问题时该看哪里。

如果角色的 `Attack` 现在不变，首先该问的不是“`MMC_CalculateAttack` 有没有代码”，而是：

- 现在到底是哪一个 GE 的哪一个 Modifier 在使用 `MMC_CalculateAttack`
- 这个 GE 现在有没有真的挂在角色 ASC 上
- 挂上去之后，这个 Modifier 有没有因为依赖属性变化而被重新计算

## Infinite GE 不是问题的根源，但也不是万能兜底

这次还有一个很容易混淆的点，就是把 `Infinite GE` 想成“只要挂着就一定会全自动联动”。

其实 `Infinite` 只说明一件事：这个 GE 是持续存在的，不会自动过期。

它不直接等于：

- 一切依赖都会自动更新
- 一切动态结构变化都会自动重建
- 一切来源属性变化都一定能正确传导

更准确的说法应该是：

`Infinite GE + 非快照捕获 + 正确的依赖注册` 这套组合，才有机会形成自动联动。

也就是说，`Infinite` 只是“持续挂着”，`MMC` 是“怎么算”，真正让它动起来的，是 GAS 内部那套依赖追踪和脏标记更新机制。

## 这次去引擎里对到的关键机制

这轮排查里，最重要的收获是确认了一件事：GAS 确实原生支持“持续 GE 上的 MMC 随非快照属性变化自动刷新”。

也就是说，之前那种“动态来源属性一加进来，老 GE 就一定没法联动”的说法太绝对了，不严谨。

在 UE 5.6 的 `GameplayAbilities` 模块源码里，和这件事直接相关的核心逻辑大致是下面这条链：

### 非快照捕获会注册依赖

当一个 GE 的 Modifier 使用了 `MMC`，而 `MMC` 又声明了一组 `RelevantAttributesToCapture`，其中某个捕获项是 `bSnapshot = false`，那么 GAS 不会只在初次应用时取一次值然后完事。

它会把这个捕获项对应的属性聚合器记成依赖。

关键点在引擎源码里能对到：

- `FGameplayEffectAttributeCaptureSpec::RegisterLinkedAggregatorCallback`
- `Agg->AddDependent(Handle)`

这意味着，这个 Active GE 会登记到“被捕获属性的聚合器”上。后面只要这个聚合器变脏，它就有机会收到通知。

### 被依赖属性变化后，会触发依赖 GE 重算

引擎里后续走的是：

- `FActiveGameplayEffectsContainer::OnMagnitudeDependencyChange`
- `Effect.Spec.CalculateModifierMagnitudes()`
- `UpdateAllAggregatorModMagnitudes(Effect)`

这一段的意义非常直接。

它不是重新应用一个新 GE，而是让已经存在的那个 Active GE，把它依赖的 Modifier Magnitude 重新算一遍，再把结果更新回对应属性聚合器里。

所以从引擎机制上说，`Infinite GE + MMC + 非快照捕获` 本来就是可以做“派生属性自动联动”的。

不然像 `Attack = Strength + WeaponAttack + EquipmentAttack` 这种设计，根本就没有落地价值。

## 那为什么这次项目里还是没反应

问题就收束到一句话了：

**不是 GAS 方向不支持，而是项目当前这条链路里，有一段没有真正打通到引擎的依赖刷新机制。**

这里面至少有两类可能性。

### 第一类情况，是来源属性虽然写了，但没有真正把对应聚合器弄脏

这类问题通常长这样：

- 动态 `AttributeSet` 已经注册进 ASC 了
- 属性值看起来也被写进去了
- 但写入方式没有真正走到 GAS 的属性聚合器更新链
- 结果就是 `MMC` 的依赖虽然理论上存在，但没有收到可触发重算的脏通知

这个方向值得重点看，因为武器和装备来源属性现在就是通过动态属性集注入的，而且是运行时写值，不是角色初始化时就完整常驻的那种静态结构。

### 第二类情况，是依赖链本身存在，但 GE 没有正确挂到当前角色状态上

比如：

- `GE_AnnyGrantedAttribute` 确实配置了 `MMC_CalculateAttack`、`MMC_CalculateDefense` 这些 Modifier
- 但当前角色激活的并不是你以为的那一个 AbilitySet
- 或者运行时另一个 GE 覆盖了它的结果
- 又或者这个 GE 的某个 Modifier 并没有真正作用到你当前看的那组属性上

这种情况表面上也会表现成“MMC 没反应”，但根因就不是属性捕获链，而是承载 `MMC` 的 GE 根本没在正确的位置工作。

## 当前项目里已经确认过的事实

这一段很适合单独记下来，因为后面继续排查时，这些结论不应该再反复怀疑。

### 明确依据

这些是已经核实过的。

1. `GE_AnnyGrantedAttribute` 资产里确实挂了多组 `MMC`，包括：
   - `MMC_CalculateAttack`
   - `MMC_CalculateCritChance`
   - `MMC_CalculateCritDamage`
   - `MMC_CalculateDefense`
   - `MMC_CalculateMaxHealth`
   - `MMC_CalculateResistance`

2. `MMC_CalculateAttack` 当前捕获的是：
   - `UAOPrimaryAttributeSet::Strength`
   - `UAOWeaponAttributeSet::WeaponAttack`
   - `UAOEquipmentAttributeSet::EquipmentAttack`

3. `MMC_CalculateDefense`、`MMC_CalculateResistance`、`MMC_CalculateCritChance` 这些类也已经改成了去读新的动态来源属性集。

4. 武器来源属性当前是在 `UAOWeaponManagerComponent` 里通过 `SetNumericAttributeBase(...)` 写到 `UAOWeaponAttributeSet` 上的。

5. 动态属性集并不是乱加的，而是通过 `UAOAbilitySystem::AcquireDynamicAttributeSet(...)` / `ReleaseDynamicAttributeSet(...)` 这套统一入口管理的。

6. 从 UE 5.6 引擎源码来看，GAS 确实支持“非快照捕获注册依赖并在依赖变化后重算 Active GE Modifier Magnitude”。

### 基于当前资料的推断

这些不是最终定论，但现在是合理怀疑点。

1. 当前项目的问题更像是“依赖刷新链没有真的触发”，而不是“MMC 写法从根上就错了”。

2. 如果武器和装备来源属性已经能从 ASC 读到正确值，但 `Attack` 等最终属性不动，那么断点很可能在：
   - 来源属性写入没有触发对应 aggregator dirty
   - 或者承载这些 MMC 的 Active GE 没有被依赖更新链带着重算

3. 如果后续确认 GAS 自动链在当前实现下不稳定，那么最小兜底方案仍然是存在的：对承载派生属性的那个 Active GE 做一次定向强刷，比如复用 `SetActiveGameplayEffectLevel` 触发引擎内部的重新计算链，而不是粗暴重做整套系统。

### 仍需确认的缺口

这一块才是后面真正要继续查的。

1. `GE_AnnyGrantedAttribute` 当前运行时对应的 `ActiveGameplayEffectHandle` 能不能稳定拿到。
2. 武器装备写入 `WeaponAttack`、`EquipmentDefense` 之后，相关 aggregator 有没有真的变脏。
3. 变脏之后，承载 `MMC` 的 Active GE 有没有走到 `OnMagnitudeDependencyChange` 这一层。
4. 如果引擎内部确实已经重算了，为什么 `UAOCombatAttributeSet` 上的最终值还是不变。

## 这次最值得记住的一点

这次问题最有价值的地方，不是“最后到底是哪一行代码错了”，而是把一个非常容易说糊涂的问题拆开了。

以后只要再遇到类似现象，都应该按下面这个顺序想：

第一步，不要一上来就说“MMC 不行”。

先确认：

- 是不是有 GE 在承载这个 MMC
- 这个 GE 是不是当前真的激活了
- 它作用的属性是不是你现在看的那组属性

第二步，不要把“属性值变化”和“结构变化”混成一件事。

要区分：

- 是已有属性值在变
- 还是运行时新加、移除了一个来源属性集

这两种变化虽然最后都可能表现成“最终属性应该变”，但排查入口并不一样。

第三步，不要只看项目代码，要回到引擎语义。

这次如果不去翻 `GameplayAbilities` 源码，很容易草率地下一个错结论：以为 `Infinite GE` 根本不支持这种派生联动。实际上不是不支持，而是要看这条依赖链有没有真的接通。

## 这份复盘最后想留下的结论

这次问题更准确的描述，不应该写成“MMC 失效”，而应该写成：

**项目里一条基于 `Infinite GE + MMC + 非快照属性捕获` 的派生属性链，在接入动态 `AttributeSet` 来源之后，出现了联动不达预期的问题。**

这句话和“MMC 不支持动态属性集”差别非常大。

前者说明方案方向仍然成立，只是链路里某一段没有接通；后者会直接把本来可用的 GAS 设计误判成错误路线，进而把后续实现带偏。

所以这次真正该记住的，不是一个拍脑袋的结论，而是一种更稳的判断方式：

先分清职责，再分清链路，最后再判断是方案问题，还是接线问题。

## 继续往前推时，目前收敛出来的改法

在确认“动态 AttributeSet 的来源结构不稳定，不能完全指望普通 dirty 链自然兜住”之后，这次方案往前收束到了一种更工程化的做法。

这套做法的关键点，不是去武器系统、正式装备系统、以后别的动态来源系统里到处手写“重算一下”，而是把这件事往上收，统一收进 `UAOAbilitySystem` 自己的动态 AttributeSet 生命周期层。

### 这版改法到底想解决什么

它想解决的不是“普通属性值变化怎么联动”，而是“来源结构发生变化之后，派生属性链如何统一协调”。

这里要继续把两件事分开看。

第一种是普通值变化，比如：

- `WeaponAttack 10 -> 20`
- `EquipmentDefense 5 -> 15`

这种变化，如果当前来源结构本身已经稳定，理论上还是应该优先让 GAS 现成的：

- 非快照捕获
- aggregator dependency
- dirty update

这条链自然去工作。

第二种才是这次真正特殊的地方，比如：

- `UAOWeaponAttributeSet` 原来不存在，现在第一次挂到 ASC 上
- `UAOEquipmentAttributeSet` 最后一个持有者释放后，被整个从 ASC 上移除

这类变化不是普通“数值变化”，而是“来源结构变化”。既然它本质上发生在 `ASC` 管理动态 AttributeSet 的层面，那最合理的收口位置就不是业务层，而是 `UAOAbilitySystem` 里现有的：

- `AcquireDynamicAttributeSet(...)`
- `ReleaseDynamicAttributeSet(...)`

### 这次不是做异步、不是做延迟，而是做同步批处理提交

这里中间有一个很容易误会的点，所以要单独记下来。

这版方案虽然不打算在 `AcquireDynamicAttributeSet(...)` / `ReleaseDynamicAttributeSet(...)` 里“当场立刻重算”，但也不是要上定时器、延迟器或者下一帧处理。

真正收敛出来的做法是：

**在同一条调用栈里，把拓扑变化先标记下来，等这一整批动态来源修改完成后，再同步提交一次统一协调。**

这个说法听起来有点绕，实际意思其实很简单。

比如一段完整流程里，可能会先发生：

1. 动态 AttributeSet 被创建或移除
2. 来源 GE 被授予或回收
3. 对应来源属性值被写入

如果在第 1 步刚发生时就立刻重算，很可能时机太早，因为后面的来源属性值甚至还没真正写进去。这时你即使强行触发派生属性重算，算出来的也只是一个中间态。

所以这版方案的重点不是“不要同步”，而是：

**不要在结构变化刚发生的那个瞬间同步重算，而要在这一批修改完成的尾部做一次同步提交。**

### Acquire / Release 具体只做到什么程度

当前收束后的边界很清楚。

`AcquireDynamicAttributeSet(...)` 不直接负责重算，它只负责：

1. 维持同一个 ASC 下同类动态 AttributeSet 的共享实例语义
2. 判断这次是不是发生了真正的 `无 -> 有`
3. 如果是，就只标记一次“动态来源拓扑已脏”

这里的“无 -> 有”不是指 holder 数量加 1，而是指：

- 之前这个 `AttributeSetClass` 在当前 ASC 上根本没有动态实例
- 这次真的创建了实例并挂入了 ASC

同样地，`ReleaseDynamicAttributeSet(...)` 也不直接负责重算，它只负责：

1. 维持现有 holder 释放语义
2. 判断这次是不是发生了真正的 `有 -> 无`
3. 如果是，就只标记一次“动态来源拓扑已脏”

这里的“有 -> 无”也不是指 holder 数量减 1，而是指：

- 这是最后一个 holder
- 对应动态 AttributeSet 实例真的被从 ASC 上移除了

换句话说，`Acquire / Release` 本轮方案里只是拓扑边界检测器，不是同步重算器。

### 真正的协调放在哪里

真正的协调，会统一放进 `UAOAbilitySystem` 内部的一个同步批处理入口里。

当前讨论出来的语义是：

1. `ASC` 内部维护一个“当前是否处于动态来源拓扑修改批次中”的状态
2. `Acquire / Release` 只负责把“拓扑已脏”记下来
3. 当这一批修改结束时，如果发现拓扑已脏，就立刻在同一调用栈尾部同步执行一次：
   - `ReconcileDerivedAttributeGameplayEffects()`

这一步不是延迟到下一帧，也不是计时器回调，而是当前修改批次结束后立刻提交。

### 派生属性 GE 怎么识别

当前也已经收束出一个很明确的边界：

不要写死 `GE_AnnyGrantedAttribute`。

因为这次做的不是“Anny 角色特供修补”，而是：

**ASC 对一类“派生属性持续 GE”的统一协调能力。**

所以后续更合适的做法是，给这类负责汇总最终属性的 GE 一个统一标识，比如：

- GameplayEffect 资产 Tag
- 或统一的分类 Tag

这样 ASC 在做协调时，只需要：

1. 扫当前 ASC 上的 Active Gameplay Effects
2. 找到所有带这个“派生属性 GE 标识”的 GE
3. 对它们统一做一次内部重算协调

这比把代码写死成只认某个具体 GE 类名稳定得多，也更符合后续扩展方向。

### 这版方案做到什么程度，哪些东西故意不做

当前这版收口，目标是做到下面这些：

1. 业务层不感知派生属性重算
2. 动态 AttributeSet 的结构变化由 ASC 生命周期层统一感知
3. `无 -> 有 / 有 -> 无` 这两个边界会触发一次同步批处理协调
4. 派生属性 GE 通过统一标识识别，不绑死具体角色资产

同时也刻意不做下面这些扩散：

1. 不在 `AOWeaponManagerComponent`、`AOFormalEquipmentManagerComponent` 里散着补“重算调用”
2. 不把所有普通属性值变化都强制走一遍派生属性 GE 重算
3. 不把动态来源属性重新塞回角色常驻 AttributeSet
4. 不重做伤害执行链和最终属性体系

### 这版改法的本质

如果最后把这版方案压成一句话，它的本质其实很清楚：

**不是不做重算，而是不让业务层知道重算；不是每个系统自己去补，而是让 ASC 在动态 AttributeSet 拓扑变化这一层统一接住它。**

这也是这次讨论一路收束到最后，最符合“系统化优化”方向的一版方案。

---

## 继续往前落代码时，这一轮最终怎么收

为了避免后面继续把讨论拉散，这里把已经拍板的落地方式再压成一版“可直接编码”的收口结论。

### 明确依据

1. `MMC` 本身不是问题根源，`Infinite GE + 非快照捕获` 在 GAS 里理论上可以联动。
2. 当前真正不稳定的是“动态 AttributeSet 的来源结构会运行时出现和消失”。
3. 这种变化属于 ASC 运行时拓扑变化，不该由武器或正式装备业务层各自补刷新。

### 基于当前资料的推断

当前更稳的收口方式不是放弃现有 `MMC + 派生 GE`，而是：

1. 保留现有派生属性计算链。
2. 把“拓扑变化后的协调”集中放到 `UAOAbilitySystem`。
3. 在 `AbilitySet` 整批授予 / 回收结束时，再同步触发一次派生属性 GE 协调。

### 仍需确认的缺口

这一轮代码写完后，仍然需要资产侧确认：

1. 负责汇总最终属性的派生 GE，是否真的带上了 `GameplayEffect.DerivedAttributes` 标签。
2. 武器与装备新的来源属性值，是否都已经不再依赖那条旧的 `SetNumericAttributeBase(...)` 裸写路径。
3. 具体策划资产里，动态来源属性是否已经完整迁移到对应的动态 `AttributeSet`。
