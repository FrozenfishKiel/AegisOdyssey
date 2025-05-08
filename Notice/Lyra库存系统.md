# Lyra的库存系统

Lyra存在ULyraInventoryItemInstance，这个是物品类真正实例出来的对象，其内部包含一个ULyraInventoryDefinition，LyraInventoryDefinition里面又有各种UInventoryFragment用来记录信息



物品的增删改查由ULyraInventoryManager进行管理，添加物品的方式为添加Definition，而内部负责由此Definition创建对应的ItemInstance

![30a3a1a3-88b9-40a5-be42-c1d0b644252e](file:///C:/Users/frozenfish/Pictures/Typedown/30a3a1a3-88b9-40a5-be42-c1d0b644252e.png)

参数导入对应的<u>Definition</u>，然后开辟内存，构建一个Instance物品实例出来





需要注意的是，在构建的时候![4d04a834-f96b-4d08-a524-c060cf980a4a](file:///C:/Users/frozenfish/Pictures/Typedown/4d04a834-f96b-4d08-a524-c060cf980a4a.png)让instance储存了当前用于创建的类模板，这个保存方式后面会用到

![b67c1c03-9b72-45d1-90a6-e3699ee4560f](file:///C:/Users/frozenfish/Pictures/Typedown/b67c1c03-9b72-45d1-90a6-e3699ee4560f.png)

此外，lyra储存物品实例并不是储存Instance，Instance外层还有一个结构体进行包装，这个结构体继承自FFastArraySerializer，主要用于网络复制,FastArraySerializer用于容器复制管理，FFastArraySerializerItem负责管理复制容器的具体元素，而FLyraInventoryList继承自FFastArraySerializer，FLyraInventoryEntry继承自FFastArraySerializerItem

![c81b8d86-da67-4c11-ae32-38072adb7b6b](file:///C:/Users/frozenfish/Pictures/Typedown/c81b8d86-da67-4c11-ae32-38072adb7b6b.png)



![3342cf99-f97d-43d6-b67f-7ef48495c93f](file:///C:/Users/frozenfish/Pictures/Typedown/3342cf99-f97d-43d6-b67f-7ef48495c93f.png)

添加物品的方式有两种，没错，AddEntry核心函数需要有人调用，这两种方式分别是AddItemDefinition和AddItemInstance，而AddEntry函数又保存在每个被FLyraInventoryList结构体里

也就是说，每一次添加物品，需要传入对应的类模板和数量，然后调用已经在manager里面创建好的一个FLyraInventoryList对象，然后用这个对象去调用函数AddEntry，在AddEntry里面开辟一块空间去创建InventoryInstance，InventoryInstance储存在Entries里，配置好Fragment，最后将实例返回

![5cf9304e-f7ed-4de6-a9a5-17ceb68856e9](file:///C:/Users/frozenfish/Pictures/Typedown/5cf9304e-f7ed-4de6-a9a5-17ceb68856e9.png)

AddReplicatedSubObject负责为该组件内部添加一个对象用于复制，然后这个组件就支持了该对象的复制

删除也是同理，需要注意的是，无论是删除还是添加，最后都要标注MarkArrayDirty，这样就能将FFastArraySerializer结构体内部的数组标记为脏（更改），从而使复制生效

![4f461bb2-5853-4a5b-9a13-6c427e609a76](file:///C:/Users/frozenfish/Pictures/Typedown/4f461bb2-5853-4a5b-9a13-6c427e609a76.png)

#### LyraInventoryItemDefinition做了什么？

此类内部包含类ULyraInventoryItemFragment，是用来保存物品信息的类，其内部创建了一个虚函数，表示当物品对象实例创建的时候，会调用这个函数

![5a56a3ff-32ec-4bf1-a2dc-c762abdd7d00](file:///C:/Users/frozenfish/Pictures/Typedown/5a56a3ff-32ec-4bf1-a2dc-c762abdd7d00.png)

然后，在ULyraInventoryItemDefinition的部分，存储着物品信息的类，其具体的信息内容长度，也就是数组长度，就可以自定义默认参数了

![c35b2152-037c-480b-ba4e-957279323130](file:///C:/Users/frozenfish/Pictures/Typedown/c35b2152-037c-480b-ba4e-957279323130.png)

回到manager添加物品的函数，因为物品信息是默认写好的，所以我们只需要获取默认的对象就能拿到，这里的模板为父模板，但导入的参数一定是子类（因为父类只定义了虚函数但没有在父类实现所以没有意义），这里直接循环从数组中获取对象

![ef8d35e7-fc8c-435f-ab34-c0a810aec25e](file:///C:/Users/frozenfish/Pictures/Typedown/ef8d35e7-fc8c-435f-ab34-c0a810aec25e.png)

## 装备部分Equipment

Equipment部分和Inventory一样，但Fragment在inventory已经定义好，所以Equipment直接沿用，因此Equipment部分只有Definition和instance，装备涉及到穿装备和脱装备两个核心函数，这两个核心函数都做成了蓝图函数，

![17cc7afe-8b4d-40b6-97b2-92b6dce6408d](file:///C:/Users/frozenfish/Pictures/Typedown/17cc7afe-8b4d-40b6-97b2-92b6dce6408d.png)

穿装备的函数发生在PostReplicatedAdd里，这个函数在客户端接收到数组元素的添加操作**后**被调用，用于<u>初始化</u>新增元素。


