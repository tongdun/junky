[![forthebadge](https://forthebadge.com/images/badges/open-source.svg)](https://forthebadge.com)
[![forthebadge](https://forthebadge.com/images/badges/made-with-c-plus-plus.svg)](https://forthebadge.com)

## Junky plugin for GCC

----------------------------------------------------------------------------
* Junky plugin For GCC

* Inject random junk instructions during 'GIMPLE' compilation pass
* Full of black magic so do not modify unless necessary
* ABSOLUTELY NO WARRANTY so use it at your own risk

* This plugin supports GCC >= 4.8. Higher version might work as well.

* xxxzzz@2022-10
----------------------------------------------------------------------------


## Junky GCC插件

这是一个GCC编译器扩展插件，用于保护Linux平台下的二进制可执行文件(ELF)的关键逻辑，防止被轻易破解

用户在编译业务程序时通过GCC编译参数启用该插件，详情参考下面的使用方式

其会在编译器中端阶段自动向用户的原始代码流中注入各类无效指令和无效函数跳转

从而迷惑破解人员的静态分析思路并有效干扰其运行时分析过程，使其无法有效的获取业务程序的真实意图

同时适用于普通可执行程序和动态库（Shared Object），支持x86-64和AAarch64架构

## 背景

当前Linux平台并没有一个可靠的类似工具，Github里的相关项目基本处于废弃或不可用状态

此外还有极少数商用软件可以实现此功能（例如CodeMorph C/C++ Code Obfuscator）

因此我们开发出此工具并贡献到开源社区，当前仅支持Linux，不支持MacOS

## 优势

1. 编译时生成干扰指令，编译完成后无任何依赖，效率高且稳定可靠
2. 对程序性能影响较小且用户可通过参数控制干扰指令的数量
3. 用户可指定仅对某些函数进行处理
4. 扩展性强，高级开发者可自定义更多的干扰指令类型

## 授权协议

由于这是一个GCC插件，按照其规范要求，本插件为GPL协议授权

## 使用方式

用户编译自己的代码模块时在命令行或Makefile中加入下列参数，注意不要换行：

```
    -fplugin='LIB_PATH/junky-x64.so'\
    -fplugin-arg-junky-junknum=1000\
    -fplugin-arg-junky-junkpfx='test_1,test_abc,...'\
    -fplugin-arg-junky-verbose=1
```

其中：

* ```junknum``` 最大可生成的干扰指令数量

* ```junkpfx``` 目标的函数列表（基于前缀匹配），逗号分隔

* ```verbose``` 是否输出详细日志，默认开启，0表示关闭

特别需要注意的是，对于经过本插件处理的源码模块**必须关闭**GCC优化，只能以```-O0```方式编译

但同一个项目中其它模块则没有要求，例如项目里有abc1.c abc2.c abc3.c三个源码模块，
其中abc2.c中包含了敏感逻辑，则用户仅需要关闭abc2模块的优化参数

本仓库中自带的两个预编译好的插件文件是基于GCC4.8版本构建，在更高版本GCC中可能能运行

## 简单测试

仓库自带了一个```test.c```的测试程序，执行```make test```来构建

用户可执行```objdump```命令对生成的```test```文件进行反汇编来观察最终效果

例如```objdump -d -Mintel test | less```

此外我们使用该插件对Redis6.0进行构建，编译结果通过了其自带的64项测试

#### 单元测试

执行```bash unittest```开始单元测试，若成功会看到```unittest OK!```

## 开发和扩展

为什么代码如此难以理解？

因为GCC设计如此！一开始GCC并未考虑插件框架，4.3版本之后才加入了插件功能，
但其跟GCC内部完全耦合，重度依赖GCC本身的各类原始数据结构和函数，
几乎没有针对插件做任何封装，换句话就是插件的开发视图和直接改造GCC源代码的视图几乎一样

此外GCC的参考文档较少，除了官方文档没有其它任何可参考的地方，有一定门槛

插件介入点在GCC的Gimple阶段，这个阶段的主要目标是将抽象语法树转换为三元组表达式序列

本插件会随机生成多个函数并gimple化，并在用户业务函数中随机插入这些函数的调用指令

同时创建多个随机全局变量（可在```.BSS```段中看到它们），并在原始代码执行流程中随机插入对这些变量的操作指令

而```extend.cxx```则是对这些随机函数和随机变量的各种扩展操作，操作越复杂迷惑性越强，
高级开发者可扩展```extend.cxx```模块中的指令构造逻辑并加入更多的指令生成

---

## usage

1. put dynamic lib file in an accessible directory
2. add the following gcc parameters in Makefile or command line

```
    -fplugin='LIB_PATH/junky.so' -fplugin-arg-junky-junknum=1000\
    -fplugin-arg-junky-junkpfx='fn_test_1,fn_test_2...'\
    -fplugin-arg-junky-verbose=1
```

3. compile and run

* ```junknum``` how many junks to be injected
* ```junkpfx``` target function name prefix list
* ```verbose``` set to 0 to disable log output

## notice

gcc optimization should be turned off for object where target functions are defined

## plugin dev

1. install essential dependency: ```gcc-plugin-devel```
2. make

## reference

You may find everything here -> https://gcc.gnu.org/onlinedocs/gcc-4.8.5/gccint/
