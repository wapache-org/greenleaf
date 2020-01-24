#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <json-c/json.h>


/*

// 其他非常好的博文
// https://www.cnblogs.com/qingergege/p/5997762.html


// test.json
{
    "cmd":"cmd1",
    "params":{
        "id":100,
        "password":"password"
    }
}

// test_new.json
{
    "cmd":"cmd1",
    "params":{
        "id":100,
        "password":"password"
        "paramsProbj":{
            "id":200,
            "password":"password2",
        }
    },
    "result":"OK"
}

 */
int main(int argc, char* argv[])
{
    // 其实`struct json_object`应该重命名为`struct json`
    // json_object 专指JSON里的object类型
    // json_array 专指JSON里的array类型
    // 这样就不会有名称上的歧义和含糊

    // 下文注释中的json_object一律指的是JSON里的object类型

    // 从文件反序列化JSON
    struct json_object * json_root = json_object_from_file("./test.json");
    printf("%s\n", json_object_to_json_string(json_root));
    
    { // 演示读取
        // 从JSON对象获取JSON对象
        struct json_object * json_cmd = json_object_object_get(json_root,"cmd");
        printf("%s\n", json_object_to_json_string(json_cmd));
        // 从JSON对象获取它的值(简单类型才有值, json_object和json_array没有)
        char* cmd = json_object_get_string(json_cmd);
        printf("cmd = %s\n", cmd);
        
        // 演示json_object的处理
        struct json_object * json_params = json_object_object_get(json_root,"params");
        printf("%s\n",json_object_to_json_string(json_params));
        
        /*从json对象中获取数据，首先获取对象，然后进行类型转换，转成int、float、string等*/
        int id = json_object_get_int(json_object_object_get(json_params,"id"));
        printf("id = %d\n", id);
        
        struct json_object * json_password = json_object_object_get(json_params,"password");
        int pwd_len = json_object_get_string_len(json_password);
        char* pwd = json_object_get_string(json_password);
        printf("pwd_len = %d\n", pwd_len);
        printf("pwd = %s\n", pwd);
    }
    
    { // 演示写入
        struct json_object * json_result_value = json_object_new_string("OK");
        json_object_object_add(json_root, "result", json_result_value);
        
        //创建一个json对象paramsProbj
        json_object  *paramsProbj = json_object_new_object();{
            json_object_object_add(paramsProbj, "id"      , json_object_new_int(200));
            json_object_object_add(paramsProbj, "password", json_object_new_string("password2"));
        }
        struct json_object * json_params = json_object_object_get(json_root,"params");
        json_object_object_add(json_params, "paramsProbj", paramsProbj);
        printf("%s\n",json_object_to_json_string(json_root));
        
        // 回写到磁盘文件
        json_object_to_file("./test_new.json",json_root);

    }
    
    //释放内存
    // 引用计数方式，只需要释放JSON树的根节点, 无需手动释放内部的对象
    json_object_put(json_root);
    
//     使用c库最关心的是内存谁来分配，谁来释放。json-c的内存管理方式，是基于引用计数的内存树(链)。
//
// 如果把一个struct json_object 对象a，add到另一个对象b上， 就不用显式的释放(json_object_put) a了, 相当于把a挂到了b的对象树上, 释放b的时候, 就会释放a。
//
// 当a即add到b上，又add到对象c上时会导致a被释放两次(double free)，这时可以增加a的引用计数(调用函数json_object_get(a))，
// 这时如果先释放b，后释放c，当释放b时，并不会真正的释放a，而是减少a的引用计数为1，然后释放c时，才真正释放a。
// 
//     理解了上面这点，使用json-c库，基本不会有什么问题了。

// 手工调用过json_object_get的地方, 一定要手工调用json_object_put来释放引用

// 内部调用了json_object_get的函数

// json_object_new_*
// json_tokener_parse_ex
// json_tokener_parse
// json_tokener_parse_verbose

// 以上函数调用了之后, 返回的对象如果没有被添加到其他对象中, 则需要手工调用json_object_put


}