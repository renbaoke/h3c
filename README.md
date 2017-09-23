h3c
===

UNIX平台中山大学东校区联网客户端。

此分支是sha1为[816a34b](https://github.com/renbaoke/h3c/commit/816a34bbe244e95116b4b85b1f07342fae7fdc0c)提交的回退分支,不包含md5验证方式。  
用于**暂时解决**慎6慎10从2017年4月开始无法通过h3c认证的问题。待彻底查明修复后，会更新主分支。此分支用作应急过渡。

编译：make

提示：交叉编译请更改Makefile的CC变量，strip会让可执行程序更小

感谢：yah3c, wireshark

h3c_without_md5_k2p_mt7621a 是我为斐讯k2p编译好的h3c执行文件，可以运行在cpu为mt7621a操作系统为OpenWrt Chaos Calmer 15.05.1的路由器上。  
如果你在使用其他路由器，想让我帮你交叉编译对应的h3c，请发问题清单给我。
