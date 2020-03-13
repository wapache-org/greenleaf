import utils from './utils.js';

// 通过各种命令获取cpu信息
var key_value_split_regexp = /\s*:\s*/,
    empty_line_regexp = /^\s*$/,
    column_split_regexp = /\s+/,
    key_value_split_regexp2 = /\s*=\s*/
;

function get_memory_info() {
    var raw_info = get_memory_raw_info();
    var info = raw_info;

    // console.log(JSON.stringify(info, undefined, 2));
    return info;
}

 // 需要root权限才能读取
function get_memory_raw_info() {

// free
// vmstat -w

    // 1. cat /proc/meminfo
    var proc_meminfo = get_memory_proc_meminfo();


    // /proc/vmstat 
    var proc_vmstat = get_memory_proc_vmstat();

    // 4. sudo dmidecode --type memory
    var dmidecode_info = get_memory_dmidecode();

    // sysctl -a 2>/dev/null | grep vm
    // ls /proc/sys/vm
    var proc_sys_vm = get_memory_proc_sys_vm();

    // 查进程占用的内存
    // sudo ps -e -o 'pid,rsz,vsz'

    // sar -r
    // sar -W

    // 整合各种来源成最终结构
    var info = {
        proc_meminfo: proc_meminfo,
        proc_vmstat: proc_vmstat,
        proc_sys_vm: proc_sys_vm,
        dmidecode_info: dmidecode_info
    };

    return info;

}


function get_memory_proc_meminfo() {
    return utils.execute_cmd_to_map("cat /proc/meminfo 2>/dev/null", key_value_split_regexp);
}

function get_memory_proc_vmstat() {
    return utils.execute_cmd_to_map("cat /proc/vmstat 2>/dev/null", column_split_regexp);
}

function get_memory_dmidecode() {
    return utils.execute_cmd_to_map2("dmidecode -q --type memory 2>/dev/null" , null, null, '\t', key_value_split_regexp);
}

function get_memory_proc_sys_vm() {
    return utils.execute_cmd_to_map("sysctl -a 2>/dev/null | grep vm", key_value_split_regexp2);
}

function get_memory_used_rate() {
    var info= utils.execute_cmd_to_map('cat /proc/meminfo | egrep "MemTotal|MemAvailable" | awk \'{print $1" "$2}\'', key_value_split_regexp);
    console.log(JSON.stringify(info, undefined, 2));
    return (100 - (100 * parseInt(info.MemAvailable) / parseInt(info.MemTotal))).toFixed(2);
}



export default {

    get_memory_info,
    get_memory_raw_info,

    get_memory_used_rate
}
