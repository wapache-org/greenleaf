/**
 * 
 * [CGI](https://en.wikipedia.org/wiki/Common_Gateway_Interface)
 * is a simple mechanism to generate dynamic content.
 * In order to use CGI, call `mg_serve_http()` function and use
 * `.cgi` file extension for the CGI files. To be more precise,
 * all files that match `cgi_file_pattern` setting in the
 * `struct mg_serve_http_opts` are treated as CGI.
 * If `cgi_file_pattern` is NULL, `**.cgi$|**.php$` is used.
 * 
 * If Mongoose recognises a file as CGI, it executes it, and sends the output
 * back to the client. Therefore,
 * CGI file must be executable. Mongoose honours the shebang line - see
 * http://en.wikipedia.org/wiki/Shebang_(Unix).
 * 
 * For example, if both PHP and Perl CGIs are used, then
 * `#!/path/to/php-cgi.exe` and `#!/path/to/perl.exe` must be the first lines
 * of the respective CGI scripts.
 * 
 * It is possible to hardcode the path to the CGI interpreter for all
 * CGI scripts and disregard the shebang line. To do that, set the
 * `cgi_interpreter` setting in the `struct mg_serve_http_opts`.
 * 
 * NOTE: PHP scripts must use `php-cgi.exe` as CGI interpreter, not `php.exe`.
 * Example:
 * 
 * ```c
 *   opts.cgi_interpreter = "C:\\ruby\\ruby.exe";
 * ```
 * 
 * NOTE: In the CGI handler we don't use explicitly a system call waitpid() for
 * reaping zombie processes. Instead, we set the SIGCHLD handler to SIG_IGN.
 * It will cause zombie processes to be reaped automatically.
 * CAUTION: not all OSes (e.g. QNX) reap zombies if SIGCHLD is ignored.
 * 
 * 
 * 
 * 如果要用python的话, 设置为:
 * cgi_pattern **.py$
 * cgi_interpreter C:\Python27\python.exe
 * 
 */

// 包含头文件之前, 定义_GNU_SOURCE可以启用一系列的编译特性, 为GCC打开大量的编译标志
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>

#include "mongoose.h"


// 头文件
// /////////////////////////////////////////////////////////
// 常量

static const size_t BLOCKSIZE = 8192;

// 常量
// /////////////////////////////////////////////////////////
// 函数原型

static int cgi_example(char* path);
static void ev_handler(struct mg_connection *nc, int ev, void *p);

// 函数原型
// /////////////////////////////////////////////////////////
// 主函数

// 参数参考`.vscode/launch.json`
int main(int argc, char** argv) 
{
    puts("进入主程序...");
    for(int i=1;i<argc;i++){
        printf("option %d: %s\n",i,argv[i]);
    }

    char* path = argv[1];
    int code = cgi_example(path);

    printf("\n退出主程序: %d\n",code);
    fflush(stdout);

    return code;
}

// 主函数
// /////////////////////////////////////////////////////////
// 函数实现

static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;

static int cgi_example(char* path)
{

  struct mg_mgr mgr;
  struct mg_connection *nc;

  mg_mgr_init(&mgr, NULL);
  printf("Starting web server on port %s\n", s_http_port);
  nc = mg_bind(&mgr, s_http_port, ev_handler);
  if (nc == NULL) {
    printf("Failed to create listener\n");
    return 1;
  }

  // Set up HTTP server parameters
  mg_set_protocol_http_websocket(nc);
  {
    s_http_server_opts.document_root = path;
    s_http_server_opts.enable_directory_listing = "false";

    s_http_server_opts.cgi_file_pattern = "**.cgi$|**.php$|**.py$";
  }

  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}

static void ev_handler(struct mg_connection *nc, int ev, void *p) {
  if (ev == MG_EV_HTTP_REQUEST) {
    mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
  }
}