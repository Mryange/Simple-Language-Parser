# Simple-Language-Parser

## build

```
mkdir build && cd build && cmake ..
make
```



## run

在工作目录下
```
./Demo  
```

会访问当前工作目录下所有的带.ycc后缀的文件


## 语法

main函数作为主入口

```C#
def main(){
    println("hello world");
    return 0;
}
```

### 基础数据类型

基础数据为整数/浮点数/字符串 
默认都是值语义


```C#
def testvar(){
    x = 23;
    println(23);
    x = 3.14;
    println(x);
    x = "hello";
    println(x);
}
```

### 引用类型
（这个引用类型更像是一个指针类型，只是自带了解引用

```C#
def testref(){
    x = 42;
    y = & x ;
    y = 114514;
    println(x);
    y = "hello";
    println(x);
}
```

### 数组类型

```C#
def testarr(){
    x = arr 23;
    println(type(x));
    x[1] = "hello";
    x[2] = 114;
    x[5] = 114514 - 114;
    println(x[5] + x[2] + x[1]);
    println(len(x));
}
```





### 控制流语句

支持 if else while for foreach