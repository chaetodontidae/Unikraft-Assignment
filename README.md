# 软件定义操作系统的全生命周期管理任务实践成果介绍

## 目录总体结构介绍

```
├── assignment1
├── assignment2
└── assignment3
```

目录下包含各个任务的完成记录，assignment1对应任务1的成果记录，如此类推

## Assignment1

```
├── notes
├── session1
├── session2
├── session3
├── session4
├── session5
├── session6
└── session7
```

assignment1 目录结果如上所示，session-X目录对应session-X的完成结果目录，主要包含各hackson中session-X的各app的相关的配置文件信息。

notes记录了unikraft起步的一些基本概念、各session的详细实现过程以及unikraft文件系统模块源码学习的记录

## Assignment2

```
├── james.org
├── libvirt
└── notes
```

james.org文件夹下包含对libvirt-go模块的基本封装，采用gin框架实现对libvirt管理虚拟机的简单创建、查询、删除、挂起和恢复基本的后端服务

libvirt文件下包含迁移nginx镜像的xml配置文件，通过该xml文件可以通过libvirt运行基本的nginx应用镜像

notes文件夹下包含对libvirt的基本介绍、迁移过程的基本记录与libvirt-go简单后端管理的基本实现

## Assignment3

```
├── apps
├── notes
```

apps文件夹下包含elfloader的基本实现，部分代码复用原app-elfloader代码，主要通过对动态链接的分析以及原代码的参考重新实现ELF动态链接功能并在此基础上添加了简易的ELF更新重加载功能（参照热部署功能进行实现，但是未实现完整的热部署功能）

notes文件夹包含对ELF文件的基本介绍、动态链接的基本介绍以及实现过程的基本介绍

