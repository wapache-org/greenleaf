# 简介

**Greenleaf**的目标是成为**Greenplum**的部署,监控,维护以及开发的辅助工具.
目前已经实现了最核心的几个底层功能, 包括web server, cgi , javascript engine等, 
项目人员可以通过编写js脚本和cgi程序不断扩充新功能, 从而满足自身需求.

# 组件

```
+-----------------------------------------+
|     RESTful API  |  WebSocket API       | <---- remote host/greenplum/postgresql/greenleaf
+-----------------------------------------+
|               Web Server                |
+-----------------------------------------+
|  cgi engine | lua engine | js engine    | 
+-----------------------------------------+
|   local storage     |       ssh         | -----> (remote or local) host/greenplum/postgresql/greenleaf
+-----------------------------------------+
```

# 依赖

目前集成了以下几个库

1. mongoose: 是一个轻量级可嵌入的WEB服务器, 支持http/https/websocket.
2. quickjs: 是一个轻量级可嵌入的JavaScript引擎, 支持ES2019规范.
4. sqlite3: 是一个轻量级可嵌入的关系型数据库
7. libpq: 是各种基于PostgreSQL的数据库的客户端.
3. mustach: 是一个C语言实现的`mustache`模板引擎.
5. json-c: 是一个C语言实现的`JSON`库.
6. lua: 是一个轻量级可嵌入的脚本引擎.

通过`mongoose`实现`Web Server`组件
通过`quickjs`实现`javascript engine`组件
通过`lua`实现`lua engine`组件
通过`mongoose`等实现`cgi engine`组件

有了`cgi/lua/js`三个引擎的支撑, 就可以通过编写`php,perl,python,lua,js`等脚本来不断增强整套系统的功能.

# 构建

`Greenleaf`是核心是一个纯C的工程, 使用`CMake`管理项目的构建, 推荐使用`Visual Studio Code`作为开发工具.

源码编译步骤

```
git clone xxx
cd greenleaf
cmake
```

# 文档

在`docs`目录下有各个模块的一些API和源码说明文档. 目前文档还非常稀缺, 在第一版基本功能稳定后再补充完善.


# 例子

在`examples`目录下有`mongoose`自带的例子和改造或新写的一些其他例子.
如果你使用的开发工具是`vscode`, 可以参考`docs/vscode/launch.json`运行和调试各个例子.

## main

这是`Greenleaf`的主程序, 目前已经具备以下功能:

1. 支持静态网页
2. 支持调用CGI程序
3. 支持集成JS脚本, 目前已经具备标准C函数和操作系统相关函数的调用封装, 以及初步做好了`libpq`的调用封装.
1. 支持给予cookie/http digest的安全认证机制

命令帮助:

```
Usages: 
    greenleaf -e qjs-modules/hello.js
    greenleaf -p 8080 -r static
    greenleaf -q qjs_modules/api_handler.js
Options:
    [-h, --help]     print this message
    [-a, --auth-domain]     the domain parameter of http digest
    [-d, --database]     the database file path
    [-e, --execute]     execute script
    [-p, --poot]     web server bingding port, default is 8000.
    [-q, --qjs-api-router]     web server api request route file, default is `qjs_modules/api_request_handler.js`.
    [-r, --root]     web server root directory, default is `static`.
    [-l, --enable-directory-listing]     if cannot find index file, list directory files, default is no.
```

`-r, --root`指定WEB Server的根目录, 存放html, js, css, image等静态资源文件.
`-e, --execute`直接执行一个脚本文件, 可以理解为`QuickJS`自带的`qjs`命令的简化版, 主要用于调试和测试脚本.
`-q, --qjs-api-router` 指定RESTful API请求的处理脚本, 所有`/api/`开头的请求都会转发到此脚本中进行处理.

其他选项暂时不做说明.

## examples/cgi_example

演示如何调用CGI程序, 本例子使用的python脚本

1. `cgi-bin/hello.py`
演示一个最简单的CGI程序

```python
#!/usr/bin/python
#coding=utf-8

print("Content-Type: text/html")
print("")
print("<h1>Hello World!</h1>")
```

1. `cgi-bin/form.py`

演示了如何接收表单参数


## examples/websocket_example

运行服务端

```bash
build/examples/websocket_example static/websocket
```

打开浏览器: http://localhost:8000 就可以看到聊天窗口了

## examples/websocket_client_example

```bash
build/examples/websocket_client_example ws://localhost:8000
```
输入内容, 回车


## examples/sqlite_example

```bash
build/examples/sqlite_example
```
输入内容, 回车


## 规划

集成以下库

1. libssh : 封装libssh使得lua/js等脚本能够直接ssh到其他远程服务器.
1. libcurl : 封装libcurl使得lua/js等脚本能够直接调用其他WEB服务.
1. xterm.js: 结合libssh实现通过浏览器访问其他远程主机, 提供堡垒机功能
1. libuv: 提升mongoose性能
3. lua-jit: 提升lua脚本性能
4. raft: 用于实现Greenleaf集群
6. djv: 用于js脚本引擎的API参数校验, 基于json schema.
5. ncurses: 提供基于文本的图形界面

完善web服务器, 支持

1. auth: cookie/basic/digest/jwt
2. websocket
3. http
4. file download/upload
5. mustache
6. cgi: python/perl/php
7. database: sqlite/postgresql


