


# 打包

在vscode中执行cmake的`build_gpmon`target, 会将gpmon需要的文件都复制到`build\gpmon`目录下

用`ldd gpmon`命令查看它的依赖

```bash
$ ldd gpmon 
	linux-vdso.so.1 (0x00007f2623d3e000)
	libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007f262348b000)
	libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007f2623287000)
	libpq.so.5 => /home/wapache/greenleaf/lib/libpq/libpq.so.5 (0x00007f262303e000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f2622c84000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f2623b1c000)
	libssl.so.1.1 => /lib/x86_64-linux-gnu/libssl.so.1.1 (0x00007f2622a18000)
	libcrypto.so.1.1 => /lib/x86_64-linux-gnu/libcrypto.so.1.1 (0x00007f262257f000)
	libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007f2622361000)

```
发现libpq.so是一个绝对路径,而不是相对路径

下载一个小工具将它改成相对路径
```bash
$ sudo apt install patchelf
$ patchelf --set-rpath lib gpmon
$ ldd gpmon
	linux-vdso.so.1 (0x00007ffee3ad3000)
	libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007f0e7f811000)
	libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007f0e7f60d000)
	libpq.so.5 => lib/libpq.so.5 (0x00007f0e7f3c4000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f0e7f00a000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f0e7fea2000)
	libssl.so.1.1 => /lib/x86_64-linux-gnu/libssl.so.1.1 (0x00007f0e7ed9e000)
	libcrypto.so.1.1 => /lib/x86_64-linux-gnu/libcrypto.so.1.1 (0x00007f0e7e905000)
	libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007f0e7e6e7000)

```
这样就修改好了, 发布的时候将lib/libpq.so.5一起发布即可.

如果是debug build出来的程序, 会携带很多符号, 可以用`strip`命令去掉, 去掉后程序的大小会有大幅度的减小
```bash
$ ll -h
总用量 4.4M
-rwxr-xr-x 1 postgres postgres 4.4M 2020-02-19 18:37:55 gpmon
drwxr-xr-x 2 postgres postgres 4.0K 2020-02-19 18:28:53 lib
drwxr-xr-x 5 postgres postgres 4.0K 2020-02-19 18:28:53 qjs_modules
drwxr-xr-x 4 postgres postgres 4.0K 2020-02-19 18:28:53 templates

$ strip gpmon

$ ll -h
总用量 1.1M
-rwxr-xr-x 1 postgres postgres 1016K 2020-02-19 18:41:14 gpmon
drwxr-xr-x 2 postgres postgres  4.0K 2020-02-19 18:28:53 lib
drwxr-xr-x 5 postgres postgres  4.0K 2020-02-19 18:28:53 qjs_modules
drwxr-xr-x 4 postgres postgres  4.0K 2020-02-19 18:28:53 templates
```



# 待整理和阅读资料

websocket的鉴权授权方案
https://www.cnblogs.com/duanxz/p/5440716.html
