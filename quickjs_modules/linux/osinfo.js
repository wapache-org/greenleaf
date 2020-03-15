import utils from './utils.js';

// 通过各种命令获取cpu信息
var key_value_split_regexp = /\s*:\s*\t*/,
    empty_line_regexp = /^\s*$/,
    column_split_regexp = /\s+/,
    ipv4_regexp = /(\d{1,3}\.){3}\d{1,3}/,
    equal_split_regexp = /\s*=\s*/,
    tab_split_regexp = /\s*\t+\s*/
;

function get_os_info(options){
    var info = {
        "uname": utils.execute_cmd("uname -a"),
        "hostname": utils.execute_cmd("hostname"),
        "releases": get_releases(),
        "groups": utils.execute_cmd_to_table("awk -F: 'BEGIN{print \"id name users\"}{print $3,$1,$4}' /etc/group", column_split_regexp, true),
        "users": utils.execute_cmd_to_table("awk 'BEGIN{print \"name:has_password:uid:gid:descritption:home_path:bin_path\"}{print $0}' /etc/passwd", key_value_split_regexp, true),
        "onlines": get_online_sessions(),
        "listenings":get_network_services()
    };

    return info;
}

function get_releases(){
    let info = utils.execute_cmd_to_map("cat /etc/*-release", equal_split_regexp);

    for (const key in info) {
        if (info.hasOwnProperty(key)) {
            const value = info[key];
            delete info[key];
            if(value.charAt(0)==='"' && value.charAt(value.length-1)==='"'){
                info[key.toLowerCase()] = value.substring(1, value.length-1);
            }else{
                info[key.toLowerCase()] = value;
            }
        }
    }

    return info;
}

function get_online_sessions(){
    return utils.execute_cmd_to_table2('LANG="en_US.UTF-8" ; w | tail -n +2', column_split_regexp, ' ', true, 8);
}


function get_network_services(){
    let lines = utils.execute_cmd_to_array('netstat -pnltu 2>/dev/null | egrep -v "^Active|^tcp6|^udp6"');
    return utils.lines_to_table(lines, ' ', 7);
}

export default {
    get_os_info,
    get_network_services,
    get_online_sessions
}

// 主机名
// hostname

// 发行版信息
// cat /etc/*-release

// lsb_release -a

// 如果lsb_release没安装centos可以用以下命令查到发行版本
// cat /etc/centos-release


// 内核信息
// uname -a

// cat /proc/version

// 列出内核模块
// lsmod


// 列出用户组
// cat /etc/group
// awk -F: '{print $3,$1,$4}' /etc/group
// awk -F: '{ if($4!="") { print $3,$1,$4 } }' /etc/group

// 列出用户
// cat /etc/passwd

// 列出普通用户
// grep "/home/" /etc/passwd | grep -v nologin

// 列出当前连接的用户
// w
// w | tail -n +2


// 列出所有单元文件
// systemctl list-unit-files

// 列出服务
// systemctl list-units --type service

// 列出网络服务
// netstat -pnltu
// netstat -pnltu | egrep -v "tcp6|udp6"

// 默认netstat需要root用户才能列出全部信息, 解决方法有两个, 一个是chmod +s , 一个是 修改 sudo 配置使得sudo netstat不用输入密码
// Not all processes could be identified, non-owned process info will not be shown, you would have to be root to see it all.
// chmod +s /bin/netstat

