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

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <json-c/json.h>
#include "mustach-json-c.h"

// 头文件
// /////////////////////////////////////////////////////////
// 常量

static const size_t BLOCKSIZE = 8192;

// 常量
// /////////////////////////////////////////////////////////
// 函数原型

char *read_file(const char *filename);

int string_begin_with(const char* str1, char* str2);
int string_end_with(const char* str1, char* str2);

int lua_example(char* path);
int mustache_example(char* template_path, const char* data_path);

// 函数原型
// /////////////////////////////////////////////////////////
// 主函数

int main(int argc, char** argv) 
{
    puts("进入主程序...");
    for(int i=1;i<argc;i++){
        printf("option %d: %s\n",i,argv[i]);
    }

    int code = 0;
    char* path = argv[1];
    if(string_end_with(path,".lua")==1){
        code = lua_example(path);
    }else if(string_end_with(path,".mtc")==1){
        char* template = read_file(path);
        char* data = argv[2];
        code = mustache_example(template, data);
    }

    printf("\n退出主程序: %d\n",code);
    fflush(stdout);

    return code;
}

// 主函数
// /////////////////////////////////////////////////////////
// 函数实现


int lua_example(char* path)
{
    /*
     * All Lua contexts are held in this structure. 
     * We work with it almost all the time.
     */
    lua_State *L = luaL_newstate();
    {
        luaL_openlibs(L); /* Load Lua libraries */

        { // 1. 加载脚本
            /* Load the file containing the script we are going to run */
            int status = luaL_loadfile(L, path);
            if (status) {
                /* If something went wrong, error message is at the top of */
                /* the stack */
                fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
                exit(1);
            }
        }

        { // 2. 添加变量, 以供脚本访问
            /*
            * Ok, now here we go: 
            * We pass data to the lua script on the stack.
            * That is, we first have to prepare Lua's virtual stack 
            * the way we want the script to receive it, then ask Lua to run it.
            */
            lua_newtable(L);    /* We will pass a table */
            {
                /*
                * To put values into the table, we first push the index, then the value, 
                * and then call lua_rawset() with the index of the table in the stack. 
                * 
                * Let's see why it's -3: In Lua, the value -1 always refers to the top of the stack. 
                * When you create the table with lua_newtable(),
                * the table gets pushed into the top of the stack. 
                * When you push the index and then the cell value, the stack looks like:
                *
                * <- [stack bottom] -- table, index, value [top]
                *                         -3    -2    -1
                * 
                * So the -1 will refer to the cell value, thus -3 is used to refer to the table itself.
                * Note that lua_rawset() pops the two last elements of the stack, 
                * so that after it has been called, the table is at the top of the stack.
                */
                for (int i = 1; i <= 5; i++) {
                    lua_pushnumber(L, i);   /* Push the table index */
                    lua_pushnumber(L, i*2); /* Push the cell value */
                    lua_rawset(L, -3);      /* Stores the pair in the table */
                }
            }
            /* By what name is the script going to reference our table? */
            lua_setglobal(L, "foo");
        }
        { // 3. 执行脚本
            /* Ask Lua to run our little script */
            int result = lua_pcall(L, 0, LUA_MULTRET, 0);
            if (result) {
                fprintf(stderr, "Failed to run script: %s\n", lua_tostring(L, -1));
                exit(1);
            }
        }
        { // 4. 读取脚本返回值
            /* Get the returned value at the top of the stack (index -1) */
            double sum = lua_tonumber(L, -1);
            printf("Script returned: %.0f\n", sum);
            lua_pop(L, 1);  /* Take the returned value out of the stack */
        }
    }
    lua_close(L);   /* Cya, Lua */

    return 0;
}

int mustache_example(char* template_path, const char* data_path)
{
    struct json_object * data;
    data = json_object_from_file(data_path);
    if (json_util_get_last_err() != NULL) {
        fprintf(stderr, "Bad json: %s (file %s)\n", json_util_get_last_err(), data_path);
        return 1;
    } else if (data == NULL) {
        fprintf(stderr, "Aborted: null json (file %s)\n", data_path);
        return 1;
    }

    int code = fmustach_json_c(template_path, data, stdout);

    return code;
}

// 函数实现
// /////////////////////////////////////////////////////////
// 工具函数实现


char* read_file(const char *filename)
{
	int f;
	struct stat s;
	char *result;
	size_t size, pos;
	ssize_t rc;

	result = NULL;
	if (filename[0] == '-' &&  filename[1] == 0)
		f = dup(0);
	else
		f = open(filename, O_RDONLY);
	if (f < 0) {
		fprintf(stderr, "Can't open file: %s\n", filename);
		exit(1);
	}

	fstat(f, &s);
	switch (s.st_mode & S_IFMT) {
	case S_IFREG:
		size = s.st_size;
		break;
	case S_IFSOCK:
	case S_IFIFO:
		size = BLOCKSIZE;
		break;
	default:
		fprintf(stderr, "Bad file: %s\n", filename);
		exit(1);
	}

	pos = 0;
	result = malloc(size + 1);
	do {
		if (result == NULL) {
			fprintf(stderr, "Out of memory\n");
			exit(1);
		}
		rc = read(f, &result[pos], (size - pos) + 1);
		if (rc < 0) {
			fprintf(stderr, "Error while reading %s\n", filename);
			exit(1);
		}
		if (rc > 0) {
			pos += (size_t)rc;
			if (pos > size) {
				size = pos + BLOCKSIZE;
				result = realloc(result, size + 1);
			}
		}
	} while(rc > 0);

	close(f);
	result[pos] = 0;
	return result;
}

/**判断str1是否以str2开头
 * 如果是返回1
 * 不是返回0
 * 出错返回-1
 * */
int string_begin_with(const char * str1,char *str2)
{
    if(str1 == NULL || str2 == NULL)
        return -1;
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    if((len1 < len2) ||  (len1 == 0 || len2 == 0))
        return -1;
    char *p = str2;
    int i = 0;
    while(*p != '\0')
    {
        if(*p != str1[i])
            return 0;
        p++;
        i++;
    }
    return 1;
}

/**判断str1是否以str2结尾
 * 如果是返回1
 * 不是返回0
 * 出错返回-1
 * */
int string_end_with(const char *str1, char *str2)
{
    if(str1 == NULL || str2 == NULL)
        return -1;
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    if((len1 < len2) ||  (len1 == 0 || len2 == 0))
        return -1;
    while(len2 >= 1)
    {
        if(str2[len2 - 1] != str1[len1 - 1])
            return 0;
        len2--;
        len1--;
    }
    return 1;
}


