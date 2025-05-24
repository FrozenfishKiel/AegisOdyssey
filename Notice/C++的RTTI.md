# C++的RTTI

全称：运行时识别，指的是某一个对象的具体类型在程序运行的时候被查询和使用



RTTI的引入为C++带来了许多新的可能性。例如，当我们在一个继承层次结构中有多个子类，并且我们想要在运行时确定一个基类指针或引用实际指向哪个子类的对象时，RTTI就派上了用场。

考虑以下示例：

`class Base {};
class Derived1 : public Base {};
class Derived2 : public Base {};`

`Base* obj = new Derived1();`

`if (typeid(*obj) == typeid(Derived1)) {
    // 执`行某些操作
}



在上述代码中，我们使用了typeid操作符（RTTI的一部分）来确定obj指向的实际对象类型。

从心理学的角度看，这种能力可以帮助我们减少不确定性和焦虑。当我们不确定某个对象的真实类型时，我们可能会感到不安。RTTI为我们提供了一种确定和安全的方法来获取这些信息，从而使我们能够更自信地编写代码。



以下是一些常见的RTTI应用场景：

* **动态类型转换**：使用`dynamic_cast`在继承层次结构中安全地转换类型。
* **类型检查**：使用`typeid`操作符检查对象的类型。
* **对象序列化**：在将对象保存到文件或网络传输中时，确定对象的实际类型。
  
  
  
  

重点：C++是一种静态类型的语言，这意味着<u>大部分类型检查都在编译时完成</u>。然而，RTTI为C++添加了一些动态类型检查的能力，这<u>使得C++成为一种同时支持静态和动态类型检查的语言。</u>



RTTI主要由两个部分组成：dynamic_cast和typeid。这两个工具为我们提供了在运行时查询和使用对象类型信息的能力。

工具    描述    用途
dynamic_cast    用于在继承层次结构中进行类型转换    安全地将基类指针或引用转换为派生类指针或引用

-----------------------------------------------------------------------------

typeid    返回一个表示对象类型的type_info对象    查询对象的类型



dynamic_cast的深入探讨
dynamic_cast是RTTI中最常用的工具之一。它允许我们在继承层次结构中进行安全的类型转换。如果转换是合法的，dynamic_cast会成功，否则它会返回nullptr（对于指针）或抛出一个异常（对于引用）。

考虑以下示例：

`class Base {};
class Derived : public Base {};`

`Base* b = new Derived();
Derived* d = dynamic_cast<Derived*>(b);`

在上述代码中，我们首先创建了一个Derived对象，并将其赋值给一个Base指针。然后，我们使用dynamic_cast尝试将Base指针转换为Derived指针。由于这种转换是合法的，dynamic_cast会成功。

从心理学的角度看，dynamic_cast为我们提供了一种安全的方法来处理不确定性。当我们不确定一个对象的类型时，我们可以使用dynamic_cast来尝试进行转换，而不必担心程序崩溃或未定义的行为。



## dynamic_cast和static_cast的区别在哪？



### **1. `static_cast`：编译时类型转换**

#### **核心特点**

* **编译时检查**：转换合法性由编译器在编译阶段决定，不进行运行时检查。

* **无运行时开销**：不依赖RTTI（运行时类型信息），转换直接基于静态类型。

* **适用范围**：
  
  * 基本类型之间的显式转换（如 `int` → `double`）。
  
  * 继承层次中的**<u>向上转换**</u>（派生类指针/引用 → 基类指针/引用）。
  
  * 非多态类型的指针/引用转换（但需有继承关系）。
  
  * 将 `void*` 转换为具体类型的指针（需确保类型一致）。
  
  * 用户自定义的隐式转换（如通过构造函数或转换运算符）。

#### **示例**



int a = 10;
double b = static_cast<double>(a);  // 基本类型转换

Base* base_ptr = static_cast<Base*>(derived_ptr);  // 向上转换（安全）

// 向下转换（危险！需开发者确保类型正确）
Derived* derived_ptr = static_cast<Derived*>(base_ptr);

## 为何无法向下转换？

### **`static_cast`的编译时特性**

* `static_cast`在**<u>编译时</u>**完成类型转换，编译器<u>仅根据静态类型</u>（声明的类型）进行转换，<u>不会检查对象的实际动态类型</u>。

* 如果基类指针<u>实际指向</u>的**不是目标派生类对象**，使用`static_cast`强制向下转换会导致未定义行为（UB），例如内存越界访问、数据损坏等。
  
  

![feeff618d9f22311bd0f113f29b8448](D:\Weixin\WeChat%20Files\wxid_b59tkxu8246w22\FileStorage\Temp\feeff618d9f22311bd0f113f29b8448.png)



```
class Point
{
public:
    Point(float xval);
    virtual ~Point();

    float x() const;
    static int PointCount();

protected:
    virtual ostream& print(ostream& os) const;

    float _x;
    static int _point_count;
};
```



类对象实例在生成的时候会顺带生成一个指向该类虚函数表的指针，伴随虚函数表的还有RTTI（图中那个type_info for point），其余表内的指针都指向该类的**虚函数成员**，每个多态对象都有一个指向其vtable的指针，称为vptr。RTTI（就是上面图中的 type_info 结构)通常与vtable关联。

**类内有虚函数时虚函数表会建立。**

`dynamic_cast`就是**利用RTTI**来执行运行时**类型检查**和**安全类型转换**。

* 首先，`dynamic_cast`通过查询对象的 vptr 来获取其RTTI（这也是为什么 dynamic_cast 要求对象有虚函数）
* 然后，`dynamic_cast`比较请求的目标类型与从RTTI获得的实际类型。如果**目标类型是实际类型或其基类**，则转换成功。
* 如果目标类型是派生类，`dynamic_cast`会检查类层次结构，以确定转换是否合法。如果在类层次结构中找到了目标类型，则转换成功；否则，转换失败。
* 当转换成功时，`dynamic_cast`返回转换后的指针或引用。
* 如果转换失败，对于指针类型，`dynamic_cast`返回空指针；对于引用类型，它会抛出一个`std::bad_cast`异常。
  
  
  
  

总结：类型安全转换，dynamic_cast 通过检查对象的 vptr 来判断对象的实际类型。如果没有虚拟函数，编译器不会为类生成 vtable，也就无法提供 RTTI 信息。因此，dynamic_cast 无法工作，因为它没有足够的信息来确定对象的真实类型。



再举一个例子

```
class A {
public:
    virtual void vfunc1();
    virtual void vfunc2();
    void func1();
    void func2();
private:
    int m_data1, m_data2;
};
 
class B : public A {
public:
    virtual void vfunc1();
    void func1();
private:
    int m_data3;
};
 
class C: public B {
public:
    virtual void vfunc2();
    void func2();
private:
    int m_data1, m_data4;
};
```

![这里写图片描述](https://i-blog.csdnimg.cn/blog_migrate/3bd3b35ecefd13afa68fd01d4facb5b0.png)


