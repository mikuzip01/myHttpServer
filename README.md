# myHttpServer

# 项目介绍

RAII

Reactor模型，non-blocking IO + one loop per thread（不完全是，线程中的客户端套接字用的是阻塞io）

# 总体结构
## class TheadPool 线程池类 
## class HttpData 处理Http数据的 

负责解析客户端发来的http数据

## class Responser 返回客户端数据的 

负责返回对应的客户端数据

# 注意事项

每个**使用ET模式**的文件描述符都应该是**非阻塞**的。如果文件描述符是阻塞的，那么读或写操作将会因为没有后续的事件而一直处于阻塞状态（饥渴状态）

使用Perl进行CGI处理时，content type和空白CRLF将由Perl进行构成而不是HTTP服务器本身

CGI的输出承担的是HTTP协议的响应部分，因此HTTP响应报头也要自己标准输出出来。

在返回非文本文件是，需要用二进制的方向在服务器上打开文件

优先队列（堆）不适合来保存时间信息。假设如下场景：客户A发起一个请求tcpA进行传输，在数据发送完成时由于设置了keep alive所以将该tcpA的文件描述符添加到时间堆并设置超时时间。然后客户A复用这条tcpA向服务器请求数据，数据传输完成后由于也设置了keep alive，因此需要更新定时器中的超时时间，但是堆并不能直接对其中的元素进行修改，否则需要重建堆，这样就失去了堆的优势

暂时不做oneshot，因为目前所有的get和post的数据都是在一个数据包中传送过来的的，没有发现被拆分的情况。如果与后客户端上传的数据链达到需要分成多个tcp分组发送时，在作oneshot的改动。在网络层，因为IP包的首部要占用20字节，所以这的MTU为1500－20＝1480。1480字节的载荷目前还够用

一次alarm调用只能引起一次SIGALRM信号！所以处理完成后需要重设定时器

pipeline[ 1 ]不设置为非阻塞，那管道一旦被写满，主线程就会被无限挂起？因为该句柄是在主线程上执行的回调(怎么判断回调函数在那个线程（进程）上执行)

定时器不正常工作的原因竟然时过期事件和当前事件的判断条件写反了-----

putenv()这个函数不能多次调用，至设设定一个的时候没有任何问题，CGI程序可以正常的接受，但设置第二个环境变量时似乎会把之前的设置给破坏，反正就是看似设置了两个环境变量但实际上并不是这样

将进程/线程将的通信管道改为TCP的socket，为程序的多机分布性打下基础。同时所以通信都采用TCP的话还可以方便调试（例如通过监控Recv-Q和Send-Q进行调试和预警）和跨语言的兼容性

线程池大小的阻抗匹配原则，根据这个原则调整线程池中线程的数目

putenv/setenv竟然不是线程安全的函数----

# 问题
## 关闭客户端链接时行为不正常

**Q**

使用close函数关闭时，服务器直接发送的RST包，浏览器显示连接被重置。

调用shotdown才能发出FIN包，但少了一次来自客户端的挥手确认，但浏览器显示正常

**原因**

在接受缓冲区没有被读完时，调用close()关闭套接字就会触发RST包

**解决方案**

在关闭之前清空接收缓冲区。没有专门清空缓冲区的系统调用，因此需要在用户态实现这个功能。

目前已经可以正常关闭了。但是用tcpdump观察是看不到从客户端发回的对服务器FIN报文的确认报文。（和之前调用showdown时的表象相同）

## 编译时显示有些常量被重复定义

**Q**

如标题

**原因**

报错的变量的定义放在了头文件中

**解决方案**

加入防御式编程的宏定义

将变量的定义放入源文件中而不是头文件中

## 客户端解析不正常

**Q**

服务器传回的静态网页，浏览器会把响应头给解析到页面上

**原因**

在okheader函数中有一次多余的头部信息发送

**解决方案**

删除多余的send

## 在CGI模块陷入死循环

**Q**

如题

**原因**

“EOF”的那个break语句没有被正常触发，原因未知

**解决方案**

通过环境变量传递参数个数的方式（getenv）代替通过scanf（stdin）

## 客户端接受PNG图像显示不正常

**Q**

如题

**原因**

使用了文本的方式读取的图片文件，导致回传的数据不正确

**解决方案**

使用二进制的方式读取文件即可

## chrome会对复用的连接发一个空的请求过来

chrome会利用复用的链接发空请求，但用户点击其他页面时不会触发对之前连接的复用。在firefox上，keep alive表现正常

## bookmarks.html的Content Length不正确

**Q**

保持持久连接报文实体必须要有正确Content-Length，这样事务处理才能正确的检测出一条报文的结束和另一条报文的结束，而bookmarks.html返回值并不正确

**原因**

原来自动导出的bookmarks并不是标准的html，头部长这样
```
<!DOCTYPE NETSCAPE-Bookmark-file-1>
<!-- This is an automatically generated file.
     It will be read and overwritten.
     DO NOT EDIT! -->
<META HTTP-EQUIV="Content-Type" CONTENT="text/html; charset=UTF-8">
```

**解决方案**

更改为标准的html就好了

## 第一次压力测试失败

**Q**

webbench使用如下测试参数`./bin/webbench -t 60 -c 1000 -2 --get -k http://127.0.0.1:12345/index.html`总共就只会响应不到100个链接

**原因**

主线程中，accept的使用方式出现问题。因为是配合epoll的ET模式的非阻塞serverSocket，但在accept的时候用的是阻塞时候的写法，没有用循环清空accpet的接受缓冲区，导致后续的epoll_wait不会被触发从而致使程序死锁

**解决方案**

epoll激活后循环调用accept

## 压力测试后程序会直接退出

**Q**

命令如下`./bin/webbench -t 5 -c 100 -2 --get http://127.0.0.1:12345/index.html`

**原因**

没有考虑到在传输运行过程中客户端意外退出的情况

**解决方案**

每一处调用了send和revc和函数和客户端进行交互的地方，都加入错误情况的判断，具体而言就是但失败时抛出异常，然后在外面捕获后关闭客户端的套接字

# TODO LIST

Done 传输浏览器指定的静态网页

Done 解析GET请求中的查询参数/将post和get的参数解析到一个统一的对象中

Doing 处理接收POST的每一个提交的字段。根据标准POST总共有3种形式，[详见这里](https://developer.mozilla.org/zh-CN/docs/Web/HTML/Element/form#attr-enctype)

    Done application/x-www-form-urlencoded：未指定属性时的默认值

    todo multipart/form-data：当表单包含 type=file 的 <input> 元素时使用此值。

    todo text/plain：出现于 HTML5，用于调试。

Done 对请求资源后缀的判断，以此来区分请求的是静态网页还是CGI

Doing 实现CGI

    todo 不需要输入数据的CGI网页

    Done 可以反射用户POST输入参数的CGI网页

todo 错误输入数据的鲁棒性

Done 长链接keep connection。keep alive的请求和相应方法[参考这里](https://developer.mozilla.org/zh-CN/docs/Web/HTTP/Headers/Keep-Alive)。改进思路：将关闭clientsocket的任务从Responser中分离出来，由一个专门的计时器来管理

todo 处理输入的URL，使其对大小写不敏感

Done 对无对应资源的URL的404处理需要重新安排

todo 日志系统

Done 能返回二进制文件

todo 根据请求的HTTP版本（1.0或者1.1），调整返回的请求头的协议版本，而不是固定的1.0或者1.1

Done CGI模块也要能设置Content Length，不然无法配合长链接使用

todo 对每个函数的工作原理的详细描述 - 输入、输出等

todo 单元测试？（很多都是成员函数，这些要怎么测？）

todo benchmark - [webbench](https://github.com/linyacool/WebBench)

todo 利用工具对程序进行性能分析

todo 把bookmark换一下，里面有些信息应该不好过审

Done 弄一个printf的调试开关

todo 每一个recv和send都要着错误处理，不然服务器的稳定性会很差（完全就取决于客户端，一旦套接字由客户端提前断开程序就会挂调）？？？？

# Benchark

## 单线程

index.html网页( 有磁盘I/O影响 )

./bin/webbench -t 60 -c 1000 -2 --get -k http://127.0.0.1:12345/index.html

服务器直接就宕机了----长链接也有问题---

`./bin/webbench -t 20 -c 20 -2 --get http://127.0.0.1:12345/index.html` 通过(开启终端信息打印)：Speed=113568 pages/min, 1230320 bytes/sec. Requests: 37856 susceed, 0 failed.




# 可能存在的BUG

fixme 在解析http数据判断换行时，固定的是CRLF，有没有可能出现只用LF进行换行的情况？

Done 在复用链接的情况下，超过15还没有传输完的时候，会被计时器给关闭掉链接
