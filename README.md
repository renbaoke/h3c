h3c
===

UNIX平台中山大学东校区联网客户端。

新增xor和md5两种校验方式切换选项，使用参数`-m md5`以执行md5验证，使用参数`-m xor`以执行原始的xor验证（该验证适用于慎6等宿舍）。  
请根据您宿舍的实际情况进行切换，不加-m参数默认使用xor验证。

编译：`make`

运行命令：`echo "你的密码xx"|/含h3c的目录xx/h3c_xx_xx -u 你的账号xx -i 路由器对应的wan口网卡例如eth1`  
使用MD5验证方式： `echo "你的密码xx"|/含h3c的目录xx/h3c_xx_xx -u 你的账号xx -i 路由器对应的wan口网卡例如eth1 -m md5`

提示：交叉编译请更改Makefile的CC变量，strip会让可执行程序更小

感谢：yah3c, wireshark

h3c_OpenWrt15.05_mt7621 是我为斐讯K2P编译好的h3c执行文件，可以运行在cpu为mt7621a操作系统为OpenWrt Chaos Calmer 15.05.1的路由器上。  
如果你在使用其他路由器，想让我帮你交叉编译对应的h3c可执行文件，请在issue中提出。
