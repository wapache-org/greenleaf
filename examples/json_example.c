#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <json-c/json.h>

void main()
{
        /*从文件中获取json对象*/
        struct json_object * json_policy_array;
        json_policy_array = json_object_from_file("./test.json");
        printf("%s\n",json_object_to_json_string(json_policy_array));
 
        /*从json对象中获取json对象*/
        struct json_object * json_cmd ;
        json_cmd = json_object_object_get(json_policy_array,"cmd");
        printf("%s\n",json_object_to_json_string(json_cmd));
 
        /*从json对象中获取json对象*/
        struct json_object * json_params ;
        json_params = json_object_object_get(json_policy_array,"params");
        printf("%s\n",json_object_to_json_string(json_params));
 
        int id = -1;            //用于保存账号的值
        int pwd_len = 0;
        char *pwd = NULL;       //用于保存密码的值
        char *cmd = NULL;       //用于保存cmd的值
 
        /*从json对象中获取数据，首先获取对象，然后进行类型转换，转成int、float、string等*/
        id = json_object_get_int(json_object_object_get(json_params,"id"));
        printf("id = %d\n", id);
        cmd = json_object_get_string(json_object_object_get(json_policy_array,"cmd"));
        printf("cmd = %s\n", cmd);
 
        pwd_len = json_object_get_string_len(json_object_object_get(json_params,"password"));
        pwd = json_object_get_string(json_object_object_get(json_params,"password"));
        printf("pwd_len = %d\n", pwd_len);
        printf("pwd = %s\n", pwd);
 
        /*向json文件中添加数据*/
        //创建一个json对象newPobj
        char *result = "OK";
        //往json_policy_array里面添加键值对
        json_object_object_add(json_policy_array,"result",json_object_new_string(result));
 
        //创建一个json对象paramsProbj
        json_object  *paramsProbj = NULL;
        paramsProbj = json_object_new_object();
        //往paramsProbj里面添加键值对
        json_object_object_add(paramsProbj,"id",json_object_new_int(200));
        json_object_object_add(paramsProbj,"password",json_object_new_string("password2"));
        //把paramsProbj添加到newPobj对象中
        json_object_object_add(json_params,"paramsProbj",paramsProbj);
 
        //这时还是将配置信息存在内存中呢
        printf("%s\n",json_object_to_json_string(json_policy_array));
 
        //将内存中修改后的配置文件，写会到磁盘中
        json_object_to_file("./test.json",json_policy_array);
 
        //释放内存
        //引用计数方式，无需手动释放
}