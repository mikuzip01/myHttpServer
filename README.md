# myHttpServer

# 项目介绍

RAII

Reactor模型

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

todo 长链接keep connection。keep alive的请求和相应方法[参考这里](https://developer.mozilla.org/zh-CN/docs/Web/HTTP/Headers/Keep-Alive)。改进思路：将关闭clientsocket的任务从Responser中分离出来，由一个专门的计时器来管理

todo 处理输入的URL，使其对大小写不敏感

Done 对无对应资源的URL的404处理需要重新安排

todo 日志系统

Done 能返回二进制文件

todo 对每个函数的工作原理的详细描述 - 输入、输出等

todo 单元测试？（很多都是成员函数，这些要怎么测？）

# 可能存在的BUG

fixme 在解析http数据判断换行时，固定的是CRLF，有没有可能出现只用LF进行换行的情况？

