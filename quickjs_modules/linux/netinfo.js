
// 硬件信息

// dmidecode
//  lspci | grep Ethernet 


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

// ls /sys/class/net


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

// 打印网卡速率
// for eth in `ls /sys/class/net`; do echo "$eth `ethtool $eth 2>/dev/null | grep Speed`" ; done;

// 打印网卡IP地址
// ip -o addr show scope global | awk '/^[0-9]:/{print $2, $4}' | cut -f1 -d '/'
// ip -o addr show scope global | tr -s ' ' | tr '/' ' ' | cut -f 2,4 -d ' '
