# **虚幻的反射机制**



![ea67da92-f3c9-492f-836d-276e7383ab66](file:///C:/Users/frozenfish/Pictures/Typedown/ea67da92-f3c9-492f-836d-276e7383ab66.png)



虚幻引擎中通过Findfuction来查找在C++中标记的函数，这也就是为什么允许反射的函数<u>不允许重名</u>，尽管C++中的函数允许重名（函数重载）

使用反射的方式：<u>UPROPERTY,UFUNCTION</u>





标记反射时，gen.cpp会生成内容：

定义常量对象，收集信息，结构体，枚举，函数，变量会收集具体的相关信息，然后构建一个新的静态对象将其全部收集到一起

把所有的结构体都打包最后注册到UE上



![c3a1c0b0-df2b-47a6-87b3-49f677c02438](file:///C:/Users/frozenfish/Pictures/Typedown/c3a1c0b0-df2b-47a6-87b3-49f677c02438.png)

收集枚举信息



![382235d6-6d6b-471f-bce1-e3ff8556cd23](file:///C:/Users/frozenfish/Pictures/Typedown/382235d6-6d6b-471f-bce1-e3ff8556cd23.png)

再将收集好的信息和枚举本身的信息收集到参数里



![18e76883-9244-45e1-bf9a-8a69fed3d7f6](file:///C:/Users/frozenfish/Pictures/Typedown/18e76883-9244-45e1-bf9a-8a69fed3d7f6.png)

然后再构建一个对象用来保存刚刚的参数并返回UEnum对象









![b06e0af7-5f8b-47a1-9c22-4d7e1a7340c8](file:///C:/Users/frozenfish/Pictures/Typedown/b06e0af7-5f8b-47a1-9c22-4d7e1a7340c8.png)

在结构体中，静态生成结构体中一一对应的变量



![25726ecb-ec85-4b63-b2fe-9884043a768e](file:///C:/Users/frozenfish/Pictures/Typedown/25726ecb-ec85-4b63-b2fe-9884043a768e.png)

然后将结构体的成员都打包成PropPointers[]，再将其的打包成参数

![96ac478c-6326-41ba-8e2d-a3d13be3ef51](file:///C:/Users/frozenfish/Pictures/Typedown/96ac478c-6326-41ba-8e2d-a3d13be3ef51.png)

然后构建参数信息对象，并返回 

1. 通过static FCompiledInDeferStruct存储一个用于构建结构体的一个单例函数，用于在程序启动时调用。
2. 此外还会创建一个静态对象，这个对象在构造函数中调用UScriptStruct::DeferCppStructOps函数，它用来向这DeferredCppStructOps注册一个动态管理结构体构造析构的类。 







![75d2827e-20f4-4657-a975-d988896a5f04](file:///C:/Users/frozenfish/Pictures/Typedown/75d2827e-20f4-4657-a975-d988896a5f04.png)

收集了所有标记UPROPERTY变量的信息



![b11c4e5f-7295-414a-916a-5db4084a297d](file:///C:/Users/frozenfish/Pictures/Typedown/b11c4e5f-7295-414a-916a-5db4084a297d.png)

UFUCTION的部分，包含该函数的返回值



![2b571fc3-75b8-4e95-886d-92cb19caca88](file:///C:/Users/frozenfish/Pictures/Typedown/2b571fc3-75b8-4e95-886d-92cb19caca88.png)

创建一个静态对象，然后构建一个新的打包对象，并赋值返回





![079db113-f752-43ff-b89f-012e95fcb0c7](file:///C:/Users/frozenfish/Pictures/Typedown/079db113-f752-43ff-b89f-012e95fcb0c7.png)



UCLASS内部最后会将所有这些信息全部都收集到一起储存到FClassParams

![c52e44af-5c98-4417-a00c-e7fcfd0205d5](file:///C:/Users/frozenfish/Pictures/Typedown/c52e44af-5c98-4417-a00c-e7fcfd0205d5.png)

先前标记反射的枚举，结构体，UCLASS全部都注册到引擎里



通过中间文件构建静态的对象来收集信息然后注册到引擎以完成整个反射流程，也就是UHT帮我们分析了这些宏然后填到引擎里面去





总结
==

虚幻的反射构建可分为以下四个阶段：  
生成阶段：借助UHT生成UClass代码，包括UClass构造、注册属性以及函数。  
收集阶段：利用Static自动注册的方式，在模块加载时将所有UClass登记并添加入array中管理。  
注册阶段：在模块初始化时将array中所有UClass中的Function和Property注册。  
链接阶段：生成动态链接库，编辑器加载该库。
