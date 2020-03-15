import os from './osinfo.js'

test1();
// test2();
// test3();

function test2(){
    let info = os.get_network_services();
    console.log(JSON.stringify(info, undefined, 2));
}


function test1(){
    let info = os.get_os_info();
    console.log(JSON.stringify(info, undefined, 2));
}


function test3(){
    let info = os.get_online_sessions();
    console.log(JSON.stringify(info, undefined, 2));
}