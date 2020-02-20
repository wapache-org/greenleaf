

# Q:C语言结构体指针中的const成员怎么初始化

```c
typedef struct {
    const int* p;
} C;

int main()
{
    int i = 0;
    C* c = (C*)malloc(sizeof(C));
    c->p = &i; // 这句话会报错误,只读量无法被赋值，那么我该怎么做才能初始化c让它的成员p指向i呢？
    free(c);
    return 0;
}
```

```c
(int*)(c->p) = &i; // 最关键的一步。
```


```c
typedef struct {
    const char* p;
} C;

int main()
{
    char* str = "abc";
    C* c = (C*)malloc(sizeof(C));
    
    char * tmp = (char *)c->p; // 需要借助强制类型转换和临时变量来绕过const限制
    tmp = str;

    free(c);
    return 0;
}
```

C中是没有构造函数的概念的，所以妄图想C++那样在对象构造的同时对对象进行初始化是误解的，
唯一的方法就是写一个特殊的函数，用这个函数来初始化一个结构体的内存，甚至可以用memset()函数都可以，
所以你的要求已经退化成如何对结构体进行初始化了，这是很多C++程序员回头写C程序的时候，都会反复思考的问题。

补充资料:
https://stackoverflow.com/questions/9691404/how-to-initialize-const-in-a-struct-in-c-with-malloc

```c
struct deneme {
    const int a = 15;
    const int b = 16;
};


int main(int argc, const char *argv[])
{

    // 方法1
    struct deneme *mydeneme = malloc(sizeof(struct deneme));
    *(int *)&mydeneme->a = 15;
    *(int *)&mydeneme->b = 20;

    // 方法2
    struct deneme deneme_init = { 15, 20 };
    struct deneme *mydeneme = malloc(sizeof(struct deneme));
    memcpy(mydeneme, &deneme_init, sizeof(struct deneme));

    return 0;
}

```

# Q:函数如何通过参数来返回字符串

```c

int func(const char* in, char** out) // 1. out 设置为char**, 指向字符串(char*)的指针
{
    char* tmp = (char*)malloc(strlen(in)+1);
    strncpy(tmp, in, strlen(in));
    tmp[strlen(in)] = '\0';

    *out = tmp;
    return 0;
}

int main(int argc, const char *argv[])
{
    char* out = NULL;
    int code = func("abc", &out); // 2. 通过引用来传参
    printf("%s", out);
    free(out);                    // 3. 不要忘记释放内存
    return code;
}

```


# Q: char*和unsigned char*的区别



# Q: 报 error: a label can only be part of a statement and a declaration is not a statement 的原因

switch的case语句定义了新变量, 但是没有加大括号括起来^_^
