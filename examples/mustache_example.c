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

#include "json.h"
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

// 参数参考`docs/vscode/launch.json`
int main(int argc, char** argv) 
{
    puts("进入主程序...");
    for(int i=1;i<argc;i++){
        printf("option %d: %s\n",i,argv[i]);
    }

    int code = 0;
    char* path = argv[1];
    if(string_end_with(path,".mtc")==1){
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
 * 
 * 其实用memcpy就可以实现
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
 * 
 * 其实用memcpy就可以实现
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


