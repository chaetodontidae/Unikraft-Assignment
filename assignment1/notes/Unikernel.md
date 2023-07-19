# Unikernel

## 概述

### Unikernel是什么？

一种定制化的操作系统镜像，能够更简单、更快速、更迅速地部署并运行。

Unikernel与一般宏内核地区别在于Unikernel一般只具备部分的内核与地址空间以获取更高效的运行性能以及更少量的安全开销。更近一步，在云计算的高可信要求下，unikernel减少的任何一点安全缺陷都可以带来可观的系统安全性提升。

通过裁剪OS，将会根据任务类型的需要生成库与工具。Unikraft镜像兼具高性能与减少的可受攻击面。在沙盒外，Unikraft支持多种平台与架构（KVM、Xen），使得镜像具备高移植性。

### 宏内核的问题

- 在管理程序提供隔离性的背景下，上下文切换变得不再具备意义
- 在容器只运行一个程序的情况下，多重地址空间成为额外的负担
- RPC架构下进程变得不再被需要，释放调度的开销
- 对于基于UDP的应用，网络栈显得多余
- 某些时候不需要特别复杂的内存分配器

### Unikraft基本组成

- 库：Unikraft模块，库是程序的重要组成部分。库可以任意的小，一般Unikraft中的库包含的小库可以被复用，较少重复的工作
- 配置：允许用户选择定制镜像生成包含的库并进行配置。
- 构建工具：制作最终镜像的工具，编译并连接相关的模块并且根据配置生成镜像

### 库组成

粗略分成核心库与外部库。核心库一般包含操作系统的典型功能。外部库一般是工具类型的代码，比如libc与openssl等（一般意义的库），

## 设计原则

- 单个地址空间
- 模块化系统
- 单保护级别
- 静态链接
- POSIX支持
- 平台抽象

## 架构

Unikernel结合了宏内核与微内核的设计方式，在消除组件的保护机制下采用模块化的方式构建内核。

通过API设计与静态链接，可以构建更狭窄的API便捷从而提高运行效率。为了实现这样的模块化原则，Uikernel采用如下设计：

- 微库：实现Unikraft核心的API。这些API的实现采用最小依赖，不同的实现彼此可以相互替换。为外部库、应用程序提供支持、
- 系统构建：向用户提供基于Kconfig的菜单配置，允许定制镜像平台与CPU架构，甚至配置独立地微库。构建系统将会编译、链接这些微库，为每种平台生成一个二进制文件。

下图展示了Unikraft的架构。所有组分都是微库并且拥有对应的Makefile与Kconfig文件。可以被独立地加入到unikernel的构建之中。API可以被轻松地在Kconfig menu中配置，使得程序能够挑选最符合其需求的组分构建运行环境。

![All components are micro-libraries that have their own Makefile and Kconfig configuration files, and so can be added to the unikernel build independently of each other.  APIs are also micro-libraries that can be easily enabled or disabled via a Kconfig menu; unikernels can thus compose which APIs to choose to best cater to an application's need.](https://unikraft.org/assets/imgs/unikraft-architecture.svg)

Unikraft为兼容性实现了POSIX支持，可以为程序提供以下优化：

- 不再演变的稳定程序，可以去掉额外的系统调用，减少镜像的大小与内存开销，选择高效的内存分配器
- 对于性能敏感的程序尽可能契合低层的API以获取性能

## Unikernel构建过程

![img](https://unikraft.org/assets/imgs/unikraft-overview.svg)

- 配置Unikraft 应用的编译选项
- 准备外部库的源代码
- 将库编译为Unikraft代码
- 链接得到最终的镜像

![img](https://unikraft.org/assets/imgs/build_uk.svg)

![img](https://unikraft.org/assets/imgs/unikraft-build-process.svg)

