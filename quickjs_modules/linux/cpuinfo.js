import utils from './utils.js';

// 通过各种命令获取cpu信息
var key_value_split_regexp = /\s*:\s*/,
    empty_line_regexp = /^\s*$/,
    column_split_regexp = /\s+/
;

function get_cpu_info() {
    var raw_info = get_cpu_raw_info();
    var info = raw_info;

    // console.log(JSON.stringify(info, undefined, 2));
    return info;
}

 // 需要root权限才能读取
function get_cpu_raw_info() {

    // 1. /proc/cpuinfo
    var proc_cpuinfo = get_cpu_proc_cupinfo();

    // 2. /proc/stat
    var proc_stat = get_cpu_proc_stat();

    // 3. lscpu
    var lscpu_info = get_cpu_lscpu();

    // 4. sudo dmidecode --type processor
    var dmidecode_info = get_cpu_dmidecode();

    // cat /proc/uptime

    // mpstat

    // sar -u
    // sar -P

    // 整合各种来源成最终结构
    var info = {
        proc_cpuinfo: proc_cpuinfo,
        proc_stat: proc_stat,
        lscpu_info: lscpu_info,
        dmidecode_info: dmidecode_info
    };

    return info;

}

function get_cpu_proc_cupinfo() {
    return utils.execute_cmd_to_array_map("cat /proc/cpuinfo 2>/dev/null", empty_line_regexp, key_value_split_regexp);
}

function get_cpu_proc_stat() {
    var stat = utils.execute_cmd_to_table("cat /proc/stat 2>/dev/null | grep cpu ", column_split_regexp, false);
    stat.head = [
        'name',      // cpu名称
        'user',      // 用户态时间, 一般/高优先级，仅统计 nice <= 0 的进程
        'nice',      // 用户态时间, 低优先级，仅统计 nice > 0 的进程
        'system',    // 内核态时间
        'idle',      // 空闲时间, 不包含IO等待时间
        'iowait',    // I/O等待时间, 硬盘IO等待时间
        'irq',       // 硬中断时间, 
        'softirq',   // 软中断时间
        'stealstolen',     // ???, 虚拟化环境中运行其他操作系统上花费的时间（自Linux 2.6.11开始）
        'guest',     // ???, 操作系统运行虚拟CPU花费的时间（自Linux 2.6.24开始）
        'guest_nice' // ???, 运行一个带nice值的guest花费的时间（自Linux 2.6.33开始）
    ];
    /*
说明：
1、时间单位是: 节拍, 1 jiffies = [1,10] ms 
2、常用计算等式：CPU时间 = (user + nice) + system + idle + iowait + irq + softirq 
3、man手册中io​​ wait有单独说明，iowait时间是不可靠值，具体原因如下：
  1）CPU不会等待I / O执行完成，而Iowait是等待I / O完成的时间。
    当CPU进入空闲状态时，很可能会调度另一个任务执行，所以iowait计算时间偏小; 
  2）多核CPU中，iowait的计算并非某个核，因此计算每一个cpu的iowait非常困难
  3）这个值在某些情况下会减少

    */

    return stat;
}

function get_cpu_lscpu() {
    return utils.execute_cmd_to_map("/usr/bin/lscpu 2>/dev/null", key_value_split_regexp);
}

function get_cpu_dmidecode() {
    return utils.execute_cmd_to_map2("dmidecode -q --type processor 2>/dev/null" , null, null, '\t', key_value_split_regexp);
}


function get_cpu_total_used_rate() {
    return utils.execute_cmd("cat /proc/stat 2>/dev/null | egrep \"^cpu \" | awk '{print 100-(100*($5+$6)/($2+$3+$4+$5+$6+$7+$8+$9+$10+$11))}'");
}

function get_cpu_used_rates() {
    return utils.execute_cmd_to_map("cat /proc/stat 2>/dev/null | grep cpu | awk '{print $1\": \"(100-(100*($5+$6)/($2+$3+$4+$5+$6+$7+$8+$9+$10+$11)))}'", key_value_split_regexp);
}

export default {

    get_cpu_info,
    get_cpu_raw_info,

    get_cpu_proc_cupinfo,
    get_cpu_proc_stat,
    get_cpu_lscpu,
    get_cpu_dmidecode,

    get_cpu_total_used_rate,
    get_cpu_used_rates

}

// cpu使用率:

// 各个核心的使用率
// cat /proc/stat 2>/dev/null | grep cpu | awk '{print $1": "(100-(100*($5+$6)/($2+$3+$4+$5+$6+$7+$8+$9+$10+$11)))"%"}'

// 总使用率
// cat /proc/stat 2>/dev/null | egrep "^cpu " | awk '{print 100-(100*($5+$6)/($2+$3+$4+$5+$6+$7+$8+$9+$10+$11))}'
