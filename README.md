# 简介

本项目是学习C语言编程的过程中不断充实和完善的产品.

# 说明

在`docs`目录下有各个模块的一些API和源码说明文档.

在`examples`目录下有`mongoose`自带的例子和改造或新写的一些其他例子.
如果你使用的开发工具是`vscode`, 可以参考`docs/vscode/launch.json`运行和调试各个例子.

# 依赖库列表

目前集成了以下几个库

1. mongoose
2. lua
3. mustach
4. sqlite3
5. json-c


# 例子

## main

`main.c`文件中包含了两个例子, 分别演示了`lua`和`mustach+json-c`的调用

## examples/cgi_example

演示如何调用CGI程序, 本例子使用的python脚本

1. `cgi-bin/hello.py`

```python
#!/usr/bin/python
#coding=utf-8

print("Content-Type: text/html")
print("")
print("<h1>Hello World!</h1>")
```

## 规划

集成以下库

1. libuv
2. libpq
3. lua-jit
4. raft
5. ncurses
6. wjelement

开发一个web服务器, 支持

1. auth: cookie/basic/digest/jwt
2. websocket
3. http
4. file download/upload
5. mustache
6. cgi: python/perl/php
7. database: sqlite/postgresql




