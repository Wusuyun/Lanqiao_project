# Lanqiao_project
蓝桥杯嵌入式赛道历年真题代码（带详细注释）

里面包括蓝桥杯历年省赛和国赛真题的全功能实现程序，全手敲，带注释超详细

每个题目里保留以下三个文件：Core（源码）、MDK-ARM（里面包含.hex文件和keil文件）、CubeMX

![](https://w-1318410331.cos.ap-guangzhou.myqcloud.com/Lanqiao/GIt.png)

点开CubeMX先生成完整工程，用Keil5打开，直接编译烧录即可。



如果不成功请检查以下几个问题：

1、点魔法棒->Target->ARM Compiler->选择Version 6

![](https://w-1318410331.cos.ap-guangzhou.myqcloud.com/Lanqiao/MDK.png)



2、如果CubeMX生成报错，点进Keil报错没有启动文件，大概率是因为版本兼容的问题导致复制启动文件失败。我们只需要把G431的.s文件复制到和keil同一级的目录里来，重新编译一下即可解决问题。

![完整的代码这里应该有一个启动文件，如果CubeMX复制失败则需要手动复制过来](https://w-1318410331.cos.ap-guangzhou.myqcloud.com/Lanqiao/G431.s.png)那么启动文件在哪里呢？请看下面的路径。（注意请事先用CubeMX生成一遍代码，无论成功与否）

![启动文件的路径](https://w-1318410331.cos.ap-guangzhou.myqcloud.com/Lanqiao/path.png)