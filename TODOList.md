# TODO List

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