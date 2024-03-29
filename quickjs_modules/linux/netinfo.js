import utils from './utils.js';

// 通过各种命令获取cpu信息
var key_value_split_regexp = /\s*:\s*\t*/,
    empty_line_regexp = /^\s*$/,
    column_split_regexp = /\s+/,
    ipv4_regexp = /(\d{1,3}\.){3}\d{1,3}/
;

function get_network_info(options){
    var info = {
        "controllers": get_network_controller_info(),
        "interfaces": get_network_interface_info(options)
    };

    return info;
}

// 网卡控制器信息
function get_network_controller_info() {
    var raw_info = utils.execute_cmd_to_array("lspci | awk -v FS=\": \" '/Ethernet|Network controller/{print $2}'");
    var info = raw_info;

    // console.log(JSON.stringify(info, undefined, 2));
    return info;
}
// cat /proc/net/dev

// 网卡信息
function get_network_interface_info(options) {
    options = options || {};

    var if_ips = utils.execute_cmd_to_map_array("ip -o addr show scope global | awk '/^[0-9]:/{print $2, $4}' | cut -f1 -d '/'", column_split_regexp);

    var if_ipv4 = {};
    for(let key in if_ips){
        var values = if_ips[key];
        for(let i=0;i<values.length;i++){
            // console.log(values[i]);
            if(ipv4_regexp.test(values[i])){
                if_ipv4[key] = values[i];
                break;
            }
        }
    }

    var details = {};
    if(options.detail){
        var if_names = utils.execute_cmd_to_array("ls /sys/class/net");
        for(let i=0;i<if_names.length;i++){
            details[if_names[i]]=get_network_interface_detail(if_names[i]);
        }
    }

    var info = {
        "interfaces": if_names,
        "physicals": get_physical_network_interfaces(),
        "details": details,
        "ips": if_ips,
        "ipv4": if_ipv4
    };

    if(!options.detail) delete info.details;
    if(!options.ips) delete info.ips;

    // console.log(JSON.stringify(info, undefined, 2));
    return info;
}

function get_network_interface_detail(name) {
    var raw_info = utils.execute_cmd_to_map("ethtool "+name+" 2>/dev/null", key_value_split_regexp);
    var info = raw_info;

    // console.log(JSON.stringify(info, undefined, 2));
    return info;
}

// for n in `ls`; do echo "$n: `cat $n`"; done;
/**
 * 
 * @param {string} name 网卡名称
 * @param {string} filter 统计指标过滤条件, 用于egrep命令, 譬如"_bytes|_errors"
 */
function get_network_interface_statistics(name, filter) {
    let cmd = "for n in `ls /sys/class/net/"+name+"/statistics"+(filter?(' | egrep "'+filter)+'"':"")+"`; do echo \"$n: `cat /sys/class/net/"+name+"/statistics/$n`\"; done;";

    var info = utils.execute_cmd_to_map(cmd, key_value_split_regexp);
    for (const key in info) {
        if (info.hasOwnProperty(key)) {
            info[key] = parseInt(info[key]);
        }
    }

    // console.log(JSON.stringify(info, undefined, 2));
    return info;
}


// ls /sys/class/net | egrep -v "`ls /sys/devices/virtual/net | awk 'BEGIN {print \"^&\"}{print \"|\"$0 }' | tr -d '\n'`"

function get_physical_network_interfaces() {
    return utils.execute_cmd_to_array('ls /sys/class/net | egrep -v "`ls /sys/devices/virtual/net | awk \'BEGIN {print \\"^&\\"}{print \\"|\\"$0 }\' | tr -d \'\\n\'`"');
}

function get_physical_network_interface_statistics() {
    var info = {};
    let faces = get_physical_network_interfaces();
    for (let i = 0; i < faces.length; i++) {
        const face = faces[i];
        info[face] = get_network_interface_statistics(faces[i],"_bytes");

    }
    // console.log(JSON.stringify(info, undefined, 2));
    return info;
    
}

export default {

    get_network_info, 
    get_network_controller_info,
    get_network_interface_info,
    get_network_interface_detail,
    get_network_interface_statistics,

    get_physical_network_interfaces,
    get_physical_network_interface_statistics

}


// 硬件信息

// dmidecode
//  lspci | grep Ethernet 
// lspci | awk -v FS=": " '/Ethernet|Network controller/{print $2}'

// net-tools工具集

// ifconfig
// netstat
// route


// iproute2工具集
// ip


// proc文件系统

// /proc/sys/net

// cat /proc/net/dev
// 通过定时记录/proc/net/dev的内容, 计算两次采集的流量和时差, 可以计算出过去一段时间的网络流量
// 用ethtool获取到网卡speed(带宽), 就可以计算出百分比

// 全部网卡
// ls /sys/class/net
// 虚拟网卡
// ls /sys/devices/virtual/net
// 物理网卡 = 全部网卡 - 虚拟网卡
// ls /sys/class/net | egrep -v "`ls /sys/devices/virtual/net | awk 'BEGIN {print \"^&\"}{print \"|\"$0 }' | tr -d '\n'`"

// 其他文件

// /etc/sysconfig/network-scripts/
// /etc/hosts
// /etc/resolv.conf


// 其他命令

// sysctl 
// iptables
// sar -n

// iftop 
// ifup ifdown


/*
https://www.cnblogs.com/fan-yuan/p/9231501.html

监控总体带宽使用――nload、bmon、slurm、bwm-ng、cbm、speedometer和netload
监控总体带宽使用（批量式输出）――vnstat、ifstat、dstat和collectl
每个套接字连接的带宽使用――iftop、iptraf-ng、tcptrack、pktstat、netwatch和trafshow
每个进程的带宽使用――nethogs
nagiosweb------nagios
*/


// ==========================================================

// 获取网卡名称
// ls /sys/class/net

// ethtool 
// [gpadmin@dbserver1 ~]$ ethtool ens160
// Settings for ens160:
// 	Supported ports: [ TP ]
// 	Supported link modes:   1000baseT/Full  -----> 千兆
// 	                        10000baseT/Full -----> 万兆 自适应
// 	Supported pause frame use: No
// 	Supports auto-negotiation: No
// 	Supported FEC modes: Not reported
// 	Advertised link modes:  Not reported
// 	Advertised pause frame use: No
// 	Advertised auto-negotiation: No
// 	Advertised FEC modes: Not reported
// 	Speed: 10000Mb/s   >--------------------------> 万兆网卡
// 	Duplex: Full
// 	Port: Twisted Pair
// 	PHYAD: 0
// 	Transceiver: internal
// 	Auto-negotiation: off
// 	MDI-X: Unknown
// Cannot get wake-on-lan settings: Operation not permitted
// 	Link detected: yes

// 打印出所有网卡的信息
// for eth in `ls /sys/class/net`; do ethtool $eth ; done;

// 打印网卡带宽
// for eth in `ls /sys/class/net`; do echo "$eth `ethtool $eth 2>/dev/null | grep Speed`" ; done;

// 打印网卡IP地址
// ip -o addr show scope global | awk '/^[0-9]:/{print $2, $4}' | cut -f1 -d '/'
// ip -o addr show scope global | tr -s ' ' | tr '/' ' ' | cut -f 2,4 -d ' '
