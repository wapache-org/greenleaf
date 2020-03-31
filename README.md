# 简介

**Greenleaf**诞生于2020年春节, 也就是新冠病毒肆虐中国的时期, 
从1月22日开始历时30天于2月20日上传github正式开源, 
其目标是成为**Greenplum**的一个辅助管理工具. 囊括部署,监控,维护以及开发等等辅助功能.

本项目目前还处在起步阶段, 
目前只实现了最核心的几个底层功能, 包括web server, cgi , javascript engine等, 
但已经初步具备实用价值, 项目人员可以通过编写js脚本和cgi程序不断扩充新功能, 从而满足自身需求.

随着各省疫情的消退, 各地陆陆续续已经开始复工, 正式上班后此项目会投入到公司内部的项目运维中试运行, 
接受实践的检验, 期望其能早日长大, 成为一个对社会有贡献的项目!

虽然目前还达不到产品级, 不过作为学习mongoose和quickjs的一个例子也是很不错的^_^

# 组件

```
+------------------------------------------------+
|        Other Systemm    |      Browser         | 
+------------------------------------------------+
                          | 
                          V  
+------------------------------------------------+
|          RESTful API    |  WebSocket API       | <-- remote host/greenplum/postgresql/greenleaf
+------------------------------------------------+
|      |             Web Server                  |
+ cron |-----------------------------------------+
|      |cgi engine | lua engine | js engine      | 
+ tab  |-----------------------------------------+
|      | local storage    |        ssh           | --> (remote or local) host/greenplum/postgresql/greenleaf
+------------------------------------------------+
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
7. libssh : 封装libssh使得lua/js等脚本能够直接ssh到其他远程服务器.
8. xterm.js: 结合libssh实现通过浏览器访问其他远程主机, 提供堡垒机功能
9. cron: 从mongoose-os-libs-cron复制过来的cronexpr和自己写的crontab
1. libyaml: 是一个yaml文件的读写库, `greenleaf`现在支持读取yml配置文件了

* 通过`mongoose`实现`Web Server`组件
* 通过`quickjs`实现`javascript engine`组件
* 通过`lua`实现`lua engine`组件
* 通过`mongoose`等实现`cgi engine`组件

有了`cgi/lua/js`三个引擎的支撑, 就可以通过编写`php,perl,python,lua,js`等脚本来不断增强整套系统的功能.

# 构建

`Greenleaf`的核心是一个纯C的工程, 使用`CMake`管理项目的构建, 推荐使用`Visual Studio Code`作为开发工具.

源码编译步骤

开始编译前, 需要参照`libraries/README.md`的说明, 将依赖的库安装好

```
git clone https://github.com/wapache-org/greenleaf.git
cd greenleaf ; mkdir build ; cd  build
cmake ..
make
```

# 文档

在`docs`目录下有各个模块的一些API和源码说明文档. 目前文档还非常稀缺, 在第一版基本功能稳定后再补充完善.


# 例子

在`examples`目录下有`mongoose`自带的例子和改造或新写的一些其他例子.
如果你使用的开发工具是`vscode`, 可以参考`docs/vscode/launch.json`运行和调试各个例子.

## `main.c`

这是`Greenleaf`的主程序, 目前已经具备以下功能:

1. 支持静态网页
2. 支持调用CGI程序
3. 支持调用JS脚本, 目前已经具备标准C函数和操作系统相关函数的调用封装, 以及初步做好了`libpq`的调用封装.
1. 支持基于cookie/http digest的安全认证机制
1. 支持基于xterm.js+websocket+libssh实现的webssh

命令帮助:

```
Usages: 
    greenleaf -c conf/crontab.json
    greenleaf -e qjs-modules/hello.js
    greenleaf -f conf/build/greenleaf.yml
Options:
    [-c, --crontab  ]     the contab json file, e.g. conf/crontab.json
    [-d, --database ]     the embed database file, e.g. data/sqlite.db
    [-e, --execute  ]     execute the script
    [-f, --file     ]     the config file, default is conf/greenleaf.yml
    [-h, --help     ]     print this message
    [-l, --log-level]     set log level: trace,debug,info,warn,fatal,none
```

* `-c, --crontab`定时执行指定crontab文件里的job.
* `-e, --execute`直接执行一个脚本文件, 可以理解为`QuickJS`自带的`qjs`命令的简化版, 主要用于调试和测试脚本.
* `-f, --file` 解析配置文件, 根据配置文件的内容执行指定任务或服务.

webssh体验:
1. 修改`static/xterm/index.html`文件中的连接主机,端口,用户名和密码
2. `build/greenleaf -q quickjs_modules/api_request_handler.js`启动服务器
3. 浏览器访问`http://localhost:8000/xterm/index.html`
4. 稍等片刻即可通过网页访问后端的ssh

收集主机统计信息:

统计信息包括: CPU使用率/内存使用率/磁盘IO/网络IO等4个方面
收集到的信息保存在数据库的`metrics_host_${年月日}`表中.

```
build/greenleaf -d data/sqlite.db -c conf/crontab.json -l debug
```

详见 [quickjs_modules/linux/README.md](quickjs_modules/linux/README.md)

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

1. libcurl : 封装libcurl使得lua/js等脚本能够直接调用其他WEB服务.
1. libuv: 提升mongoose性能
3. lua-jit: 提升lua脚本性能
4. raft: 用于实现Greenleaf集群
6. djv: 用于js脚本引擎的API参数校验, 基于json schema.
5. ncurses: 提供基于文本的图形界面
1. https://github.com/kristapsdz/kcgi

完善web服务器, 支持

1. auth: cookie/basic/digest/jwt
2. websocket
3. http
4. file download/upload
5. mustache
6. cgi: python/perl/php
7. database: sqlite/postgresql

# 后记

病疫无情，人间有爱。

感谢广大医务工作者在中国最困难的时期, 为我们负重前行, 
让我们向白衣天使们致敬，共同祝愿他们春暖花开日、平安归来时！

2020-03-08 01:11:11 截止至今日, 国内的疫情已经基本控制住了, 但是国外又蔓延了, 心塞!

2020-03-16 00:18:24 截止至今日, 只能用席卷全球来形容了!

2020-03-22 23:47:03 截止至今日, 支援武汉的医护人员已经完成使命了, 国内基本无新增病例, 可是国外疫情却越来越糟糕了!

2020-04-01 00:36:37 截止至今日, 国外疫情依然没有好转的迹象!
