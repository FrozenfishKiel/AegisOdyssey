# Lyra的GAS系统-属性

## AttributeSet

### 基本配置：

前面（[# Lyra的GAS系统-技能-CSDN博客](https://blog.csdn.net/qq_43342791/article/details/148351909?spm=1001.2014.3001.5501)）我们说到Lyra的技能配置，其实**属性配置**也是和技能配置在同一个地方，也就是LyraAbilitySet和**GameFeatureAction_AddAbilities**里，载入ASC的操作也在这两个类中完成



![4813e5c7-75bf-4df7-b8de-8f240a0b1b53](file:///C:/Users/frozenfish/Pictures/Typedown/4813e5c7-75bf-4df7-b8de-8f240a0b1b53.png)

然后接下来是AttributeSet类的封装，Lyra创建了一个LyraAttributeSet，这作为Lyra属性集的一个基类，之后分开的一些属性集例如LyraHealthSet ， LyraCombatSet都是继承于它

![58717bff-cf58-472a-997c-cd882273ec28](file:///C:/Users/frozenfish/Pictures/Typedown/58717bff-cf58-472a-997c-cd882273ec28.png)

宏的设置，编译时会默认给变量生成相应的**Getter**以及**Setter**函数，当前设置**会生成四个函数，获取属性，获取值，设置值，以及初始化值**。

下方的委托是为了处理游戏效果改变的属性时要广播的信息，稍后会说

接下来是子类的实现，我们跳转到LyraHealthSet的头文件，可以看到

![8e97694a-8486-4d3a-b7c4-d40fe616ae84](file:///C:/Users/frozenfish/Pictures/Typedown/8e97694a-8486-4d3a-b7c4-d40fe616ae84.png)

Lyra定义了两个基本属性，Health和Maxhealth，也就是我们数值的当前生命值和最大生命值，以及两个**元属性**，回复生命值和受到的伤害，元属性的作用是他会被复杂的计算得出，最后作用与相应的属性，例如最终伤害 = 敌人的攻击力 + 敌人的穿甲 举个例子，计算的过程根据实际情况决定，总之这个“最终伤害”就是元属性，元属性只需要在服务器计算在服务器修改属性就行，不需要复制。

![b92cbbc5-6988-46b3-a565-112910c1f777](file:///C:/Users/frozenfish/Pictures/Typedown/b92cbbc5-6988-46b3-a565-112910c1f777.png)

以上定义的四个宏就是为这四个属性生成四个函数，获取属性，获取值，设置值，以及初始化值，而下方则是刚刚说的委托定义，当对应的属性发生变化时会进行广播，另外，别忘了**属性定义的时候也设置了复制回调函数**，表明这些属性都是会复制给客户端的。

### 使用方式：

然后来到CPP

```
void ULyraHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(ULyraHealthSet, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(ULyraHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
}
```

Attribute 需要被添加到GetLifetimeReplicatedProps中，COND_None 为触发没有条件限制，REPTNOTIFY_Always 告诉 OnRep 方法在本地值和服务器下发的值即使已经相同也会触发（为了预测），默认情况下OnRep不会触发

```
void ULyraHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(ULyraHealthSet, Health, OldValue);

    // Call the change callback, but without an instigator
    // This could be changed to an explicit RPC in the future
    // These events on the client should not be changing attributes

    const float CurrentHealth = GetHealth();
    const float EstimatedMagnitude = CurrentHealth - OldValue.GetCurrentValue();

    OnHealthChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);

    if (!bOutOfHealth && CurrentHealth <= 0.0f)
    {
        OnOutOfHealth.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentHealth);
    }

    bOutOfHealth = (CurrentHealth <= 0.0f);
}
```

这里只摘取了生命值变化的客户端回调函数，后面的**属性客户端回调函数逻辑都是大同小异**的；变量的OnRep 函数调用GAMEPLAYATTRIBUTE_REPNOTIFY 宏才能使用预测系统，同时，mutable FLyraAttributeEvent OnOutOfHealth;委托广播也在客户端执行，目的是将内部信息传递到客户端，通常是传递到UI（客户端独有）上，bOutOfHealth变量是用于跟踪当前生命值达到0的情况，由上面的逻辑不难看出这一点。

```
bool ULyraHealthSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData &Data)
{
    if (!Super::PreGameplayEffectExecute(Data))
    {
        return false;
    }

    // Handle modifying incoming normal damage
    if (Data.EvaluatedData.Attribute == GetDamageAttribute())
    {
        if (Data.EvaluatedData.Magnitude > 0.0f)
        {
            const bool bIsDamageFromSelfDestruct = Data.EffectSpec.GetDynamicAssetTags().HasTagExact(TAG_Gameplay_DamageSelfDestruct);

            if (Data.Target.HasMatchingGameplayTag(TAG_Gameplay_DamageImmunity) && !bIsDamageFromSelfDestruct)
            {
                // Do not take away any health.
                Data.EvaluatedData.Magnitude = 0.0f;
                return false;
            }

#if !UE_BUILD_SHIPPING
            // Check GodMode cheat, unlimited health is checked below
            if (Data.Target.HasMatchingGameplayTag(LyraGameplayTags::Cheat_GodMode) && !bIsDamageFromSelfDestruct)
            {
                // Do not take away any health.
                Data.EvaluatedData.Magnitude = 0.0f;
                return false;
            }
#endif // #if !UE_BUILD_SHIPPING
        }
    }

    // Save the current health
    HealthBeforeAttributeChange = GetHealth();
    MaxHealthBeforeAttributeChange = GetMaxHealth();

    return true;
}
```

PreGameplayEffectExecute字面意思是**效果应用前的处理**，它的作用是当某一个效果应用到当前的AttributeSet时，会触发这个函数，当函数返回false时，会**阻止效果的应用**，反之。这段逻辑的目的是检查当前的伤害应用是否成立：如果目标有`TAG_Gameplay_DamageImmunity`标签，且伤害来源不是自毁（`TAG_Gameplay_DamageSelfDestruct`），将伤害值设为0并返回`false`，阻止伤害效果应用。如果是return true，也就是允许效果应用时，在此之前，用两个常量**HealthBeforeAttributeChange** 和 **MaxHealthBeforeAttributeChange** 保存当前的生命值和最大生命值，这两个变量会在待会使用

```
void ULyraHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    const bool bIsDamageFromSelfDestruct = Data.EffectSpec.GetDynamicAssetTags().HasTagExact(TAG_Gameplay_DamageSelfDestruct);
    float MinimumHealth = 0.0f;

#if !UE_BUILD_SHIPPING
    // Godmode and unlimited health stop death unless it's a self destruct
    if (!bIsDamageFromSelfDestruct &&
        (Data.Target.HasMatchingGameplayTag(LyraGameplayTags::Cheat_GodMode) || Data.Target.HasMatchingGameplayTag(LyraGameplayTags::Cheat_UnlimitedHealth) ))
    {
        MinimumHealth = 1.0f;
    }
#endif // #if !UE_BUILD_SHIPPING

    const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
    AActor* Instigator = EffectContext.GetOriginalInstigator();
    AActor* Causer = EffectContext.GetEffectCauser();

    if (Data.EvaluatedData.Attribute == GetDamageAttribute())
    {
        // Send a standardized verb message that other systems can observe
        if (Data.EvaluatedData.Magnitude > 0.0f)
        {
            FLyraVerbMessage Message;
            Message.Verb = TAG_Lyra_Damage_Message;
            Message.Instigator = Data.EffectSpec.GetEffectContext().GetEffectCauser();
            Message.InstigatorTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
            Message.Target = GetOwningActor();
            Message.TargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
            //@TODO: Fill out context tags, and any non-ability-system source/instigator tags
            //@TODO: Determine if it's an opposing team kill, self-own, team kill, etc...
            Message.Magnitude = Data.EvaluatedData.Magnitude;

            UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
            MessageSystem.BroadcastMessage(Message.Verb, Message);
        }

        // Convert into -Health and then clamp
        SetHealth(FMath::Clamp(GetHealth() - GetDamage(), MinimumHealth, GetMaxHealth()));
        SetDamage(0.0f);
    }
    else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
    {
        // Convert into +Health and then clamo
        SetHealth(FMath::Clamp(GetHealth() + GetHealing(), MinimumHealth, GetMaxHealth()));
        SetHealing(0.0f);
    }
    else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        // Clamp and fall into out of health handling below
        SetHealth(FMath::Clamp(GetHealth(), MinimumHealth, GetMaxHealth()));
    }
    else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
    {
        // TODO clamp current health?

        // Notify on any requested max health changes
        OnMaxHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxHealthBeforeAttributeChange, GetMaxHealth());
    }

    // If health has actually changed activate callbacks
    if (GetHealth() != HealthBeforeAttributeChange)
    {
        OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
    }

    if ((GetHealth() <= 0.0f) && !bOutOfHealth)
    {
        OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
    }

    // Check health again in case an event above changed it.
    bOutOfHealth = (GetHealth() <= 0.0f);
}
```

PostGameplayEffectExecute则是**效果应用后会调用**，这段代码的逻辑是作弊保护检测目标的游戏模式防止死亡，对伤害的处理，将传入的Damage值转为Health减少，并重置Health，传入的Healting转为生命值增加，重置Healthing；Clamp确保Health的值始终处于[0,最大生命值]

范围内，然后就是广播信息，若最大生命值更新，则触发OnMaxHealthChange事件。



**UGameplayMessageSubsystem**是Lyra的一个插件，具体的使用方式请查看[无需引用actor，实现actor通信（Lyra的Gameplay Message Subsystem插件学习分享）_哔哩哔哩_bilibili](https://www.bilibili.com/video/av385115332/?vd_source=601bb608de0af64687074514cae74116)

如果不想使用，就自己替换成传统的**观察者模式**

```
void ULyraHealthSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
    Super::PreAttributeBaseChange(Attribute, NewValue);

    ClampAttribute(Attribute, NewValue);
}

void ULyraHealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    ClampAttribute(Attribute, NewValue);
}

void ULyraHealthSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);

    if (Attribute == GetMaxHealthAttribute())
    {
        // Make sure current health is not greater than the new max health.
        if (GetHealth() > NewValue)
        {
            ULyraAbilitySystemComponent* LyraASC = GetLyraAbilitySystemComponent();
            check(LyraASC);

            LyraASC->ApplyModToAttribute(GetHealthAttribute(), EGameplayModOp::Override, NewValue);
        }
    }

    if (bOutOfHealth && (GetHealth() > 0.0f))
    {
        bOutOfHealth = false;
    }
}

void ULyraHealthSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
    if (Attribute == GetHealthAttribute())
    {
        // Do not allow health to go negative or above max health.
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
    }
    else if (Attribute == GetMaxHealthAttribute())
    {
        // Do not allow max health to drop below 1.
        NewValue = FMath::Max(NewValue, 1.0f);
    }
}
```

ClampAttribute强制将属性值在合理范围内，Health不可为负，且不可超过当前MaxHealth；MaxHealth最低为1，防止无效值。

PreAttributeBaseChange & PreAttributeChange（属性修改前拦截），在属性值实际修改之前，通过ClampAttribute保证合法；PostAttributeChange（属性修改后处理），在属性值修改之后，处理依赖逻辑和状态更新，当MaxHealth变化时，若当前Health超过当前MaxHealth，直接覆盖Health为MaxHealth（避免生命值溢出），例如Maxhealth从100到50->若当前生命值是80，则强制将生命值改为50，死亡状态更新：若角色之前已经死亡（bOutOfHealth = true），但生命值恢复为正数->清除死亡状态。



总之Lyra的AttributeSet的使用配置和大部分的项目一致，这里只是列举了HealthAttributeSet的配置和使用方式，但其实这也是用到了AttributeSet最常用的所有的使用方式，Lyra其他的AttributeSet也是大同小异的。


