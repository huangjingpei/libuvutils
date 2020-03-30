# libuvutils
  libuvutils　是在基于夸平台库(libuv)之上封装的一层基于线程、信号量、定时器、socket操作的一个C++库，
　代码的设计主要参考[mediaosoup](http://https://github.com/versatica/mediasoup.git)的代码，略有改。
# 版本说明
## v0.1, 2020-03-30
### 特点：
- 代码的集成编译通过
- 写了两个test函数测试了Thread, Timer类
### 下个版本改进
- 逐步完善每个模块的test,尤其对socket部分实现`TcpServer&TcpClient` 和`UdpServer&UdpClient`操作
- 对库里面的异常抛出增加控制开关
- 线程类需要增加消息投递和处理函数
### 版本讨论
- 是否有必要把std::map都换成c语言里面的链表操作
- Logger部分代码移植过来的时候已经被注释掉，是否有必要照搬他那种方式
- 讨论对函数名称进行按照`驼峰命名法`改进, 是否有必要

## 未来版本计划
- 引用智能指针RefBase类,　方便掌控对象生命周期
　

