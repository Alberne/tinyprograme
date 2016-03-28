C语言常用的数据结构接口与实现
===================

秀的软件一定有设计良好的**数据结构**和基于该结构漂亮的**算法**。**C**语言是帮助深刻理解**数据结构**和**算法**细节优秀
的语言工具，该库存提供了一些基于**C语言版数据结构与算法**的接口与实现，以及使用案例。
   
`欢迎大家fork，追加新的功能`    
______________________

####Table
>**Table(哈希表):**  是一组**key-value**对的集合，table的使用场景十分广泛。**key、value**可以是任何类型的数据。
[table.c](/C-Interface/table/table.c "table 文件夹")提供了一些常用的实现

![table](https://github.com/Alberne/tinyprograme/blob/master/C-Interface/table/img/table_2.jpg "table 结构")



####MemoryManage
>**MemoryManage(内存分配器)：** 通过**哈希表**来统一管理内存块，另外，使用*`freelist`*静态链表来管理空闲的内存块，内存块一
旦被系统分配之后，就不再释放。程序使用完后将存放到*`freelist`*链表中，以备再次使用。[mem.c](/C-Interface/MemoryManage/mem.c "")提供了常用的实现。对于反复使用内存的程序来说，该 **内存分配器**可以显著提高程序性能，而且易于管理。



![memorymanage](/C-Interface/MemoryManage/memorycalloter.jpg "快照")



###list
>**list(单链表)**:[list.c](/C-Interface/list/list.c "")提供了一些通用的实现,根据list提供的接口可以实现*`队列`*,*`栈`*,*`链表`*数据结构.

![list](/C-Interface/list/list.jpg "快照")

###set
>**set(集合)**:一种常用的数据结构，用于存放无序，并且不重复的数据.内部的数据存储方式采用的是哈希表实现，并提供了常用的几种集合运算:*`差集`*，*`交集`*，*`并集`*，*`对称差集`*

###bitset
>**bitset**:位域集合,每个集合元素用一个bit位来表示
