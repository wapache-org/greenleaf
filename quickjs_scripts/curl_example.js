import * as std from "std";

var github = {
    base: 'https://api.github.com',
    user: 'wapache-org',
    users: '/users'
};

var api = {
    user_info: github.base + github.users + '/' + github.user
}

// var result1 = std.urlGet(api.user_info);
// console.log('GET', api.user_info, result1);

var result = std.curl(api.user_info, {
    full: true,
    method: 'GET'
});
if(result.status==200){
    if(result.body){
        result.body = JSON.parse(result.body);
    }
}
if(result.headers){
    var header_array = result.headers.split("\r\n")
    .map(function (value) {
        var index = value.indexOf(": ");
        // if(index > 0){
        //     var pair = {};
        //     pair[value.substring(0,index)]=value.substring(index+2);
        //     return pair;
        // }
        // return null;
        return index>0 ? [value.substring(0,index),value.substring(index+2)] : null;
    }).filter(function (value){
        return value!=null;
    });

    var header_map = {};
    header_array.forEach(e => {
        var v = header_map[e[0]];
        if(v == undefined){
            header_map[e[0]] = e[1];
        }else if(typeof v === 'string'){
            header_map[e[0]] = [v, e[1]];
        }else if(v.push){
            v.push(e[1]);
        }else{
            console.log('Warning', e[0], e[1], v);
            header_map[e[0]] = e[1];
        }
    });

    result.headers = header_map;
}else{
    // result.headers = [];
}

console.log('GET', api.user_info, JSON.stringify(result, undefined, 2));

// build/greenleaf -e quickjs_scripts/curl_example.js

// std.curl is std.urlGet's extention. modified quickjs's source!!! 
