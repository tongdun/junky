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

这是一个GCC编译器扩展插件，用于保护Linux平台下的二进制可执行文件(ELF)的关键逻辑，防止被轻易反汇编破解

用户在编译业务程序时通过参数启用，随后插件会在编译器中端阶段自动向原始代码流中注入各类无效指令和无效函数跳转

从而迷惑破解人员的静态分析思路并有效干扰其运行时分析过程，使之无法有效的获取业务程序的真实意图

适用于普通程序和动态库，支持x86-64和AAarch64架构

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

由于这是一个GCC插件，按照其规范要求，本插件为GPL协议发布

## 使用方式

编译时在命令行或Makefile中加入下列参数（注意不要换行）：

    -fplugin='LIB_PATH/junky-x64.so' 
    -fplugin-arg-junky-junknum=1000
    -fplugin-arg-junky-junkpfx='fn_test_1,fn_test_2...'
    -fplugin-arg-junky-verbose=1

其中：

* ```junknum``` 最大可生成的干扰指令数量

* ```junkpfx``` 目标的函数列表（基于前缀匹配），逗号分隔

* ```verbose``` 是否输出详细日志，默认开启，0表示关闭

特别需要注意的是，对于经过本插件处理的源码模块**必须关闭**GCC优化，只能以```-O0```方式编译

本仓库中自带的两个二进制文件基于GCC4.8版本构建，在更高版本GCC中可能能运行

#### 简单测试

仓库自带了一个```test.c```的测试程序，执行```make test```来构建

然后用户可通过```objdump```命令对生成的```test```文件进行反汇编来观察最终效果

例如```objdump -d -Mintel test | less```

此外我们使用该插件对Redis6.0进行构建，编译结果通过了其自带的64项测试

---

## usage

1. put dynamic lib file into an accessible directory
2. add the following gcc parameters in Makefile or command line

    -fplugin='LIB_PATH/junky.so' -fplugin-arg-junky-junknum=1000
    -fplugin-arg-junky-junkpfx='fn_test_1,fn_test_2...'
    -fplugin-arg-junky-verbose=1

3. compile and run

* ```junknum``` controls how many junks to be injected
* ```junkpfx``` target function name prefix list
* ```verbose``` set to 0 to disable log output

## notice

gcc optimization should be turned off for object which contains target functions

## plugin dev

1. install essential dependency: ```gcc-plugin-devel```
2. make

## reference

You may find everything here -> https://gcc.gnu.org/onlinedocs/gcc-4.8.5/gccint/
