#### 防止自己忘了当时写这个的目的。

#### 问题描述

一条记录中具有三个关键非常重要属性，busiId（long），servId（long），type（int），而其中type通常为形如101十进制数，其中type的百位代表busiId的属性，而type的个位代表servId的属性，现在的需求是具有相同busiId和其属性或者具有相同servId及其属性的记录要分为一组。  
原有的做法是，用两个vector分别存下以busiId和type联合排序（busiId为主，type为辅，以后称B vector）后的结果和以servId和type联合排序（servId为主，type为辅，以后称S vector）后的结果。 初始化时，加入一条记录数据，记录其在两个排序结果数组的位置，先以在S vector中上下查找与刚加入记录是否含有相同的servId和type，一、如果没有查到有相同servId和type则，停止查询；二、如果查到，首先标记该记录，随后在B vector中上下查询是否有与刚加入的记录具有相同的busiId和type记录。查询结果与servId的方式类似。（深度优先遍历）  
现有想法是按照并查集的方式优化，具体代码如该文件夹中的代码所示。   
