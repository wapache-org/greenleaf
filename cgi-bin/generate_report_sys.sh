

echo "|+++++++++++++++++++++++++++++++++++++++++++++++++++++++++|"
echo "|                      操作系统信息                       |"
echo "|+++++++++++++++++++++++++++++++++++++++++++++++++++++++++|"
echo ""

echo "----->>>---->>>  主机名: "
hostname -s
echo ""
echo "----->>>---->>>  以太链路信息: "
ip link show
echo ""
echo "----->>>---->>>  IP地址信息: "
ip addr show
echo ""
echo "----->>>---->>>  路由信息: "
ip route show
echo ""
echo "----->>>---->>>  操作系统内核: "
uname -a
echo ""
echo "----->>>---->>>  内存(MB): "
free -m
echo ""
echo "----->>>---->>>  CPU: "
lscpu
echo ""
echo "----->>>---->>>  块设备: "
lsblk
echo ""
echo "----->>>---->>>  拓扑: "
lstopo-no-graphics
echo ""
echo "----->>>---->>>  进程树: "
pstree -a -A -c -l -n -p -u -U -Z
# 命令解析 
# -a 显示启动进程时的完整指令, 包括启动进程的路径,参数等
# -A
# -c 不使用精简法显示进程信息, 即显示的进程中包含子进程和父进程
# -l 
# -n 根据进程PID号来排序输出, 默认是以程序名排序
# -p 显示进程的PID
# -u 显示进程对应的用户名称
# -U
# -Z
echo ""
echo "----->>>---->>>  操作系统配置文件 静态配置信息: "
echo "----->>>---->>>  /etc/sysctl.conf "
grep "^[a-z]" /etc/sysctl.conf
echo ""
echo "----->>>---->>>  /etc/security/limits.conf "
grep -v "^#" /etc/security/limits.conf|grep -v "^$"
echo ""
echo "----->>>---->>>  /etc/security/limits.d/*.conf "
for dir in `ls /etc/security/limits.d`; do echo "/etc/security/limits.d/$dir : "; grep -v "^#" /etc/security/limits.d/$dir|grep -v "^$"; done 
echo ""
echo "----->>>---->>>  /etc/sysconfig/iptables "
cat /etc/sysconfig/iptables
echo ""
echo "----->>>---->>>  /etc/fstab "
cat /etc/fstab
echo ""
echo "----->>>---->>>  /etc/rc.local "
cat /etc/rc.local
echo ""
echo "----->>>---->>>  /etc/selinux/config "
cat /etc/selinux/config
echo ""
echo "----->>>---->>>  /boot/grub/grub.conf "
cat /boot/grub/grub.conf
echo ""
echo "----->>>---->>>  /var/spool/cron 用户cron配置 "
for dir in `ls /var/spool/cron`; do echo "/var/spool/cron/$dir : "; cat /var/spool/cron/$dir; done 
# 脚本解析
#   `ls /var/spool/cron` : /var/spool/cron以账号来区分每个用户自己的计划任务, /etc/crontab是系统计划任务, 需要在五个*后面加上用户
#   echo "/var/spool/cron/$dir : "; : 输出文件的完整路径
#   cat /var/spool/cron/$dir;       : 输出文件的内容

echo ""
echo "----->>>---->>>  chkconfig --list "
chkconfig --list
# chkconfig用于检查和设置系统的各种服务, Red Hat系才有
echo ""
echo "----->>>---->>>  iptables -L -v -n -t filter 动态配置信息: "
iptables -L -v -n -t filter
echo ""
echo "----->>>---->>>  iptables -L -v -n -t nat 动态配置信息: "
iptables -L -v -n -t nat
echo ""
echo "----->>>---->>>  iptables -L -v -n -t mangle 动态配置信息: "
iptables -L -v -n -t mangle
echo ""
echo "----->>>---->>>  iptables -L -v -n -t raw 动态配置信息: "
iptables -L -v -n -t raw
echo ""
echo "----->>>---->>>  sysctl -a 动态配置信息: "
sysctl -a
# sysctl用于运行时配置内核参数, 这些参数位于/proc/sys目录下. 如果想永久保留设置, 可以修改/etc/sysctl.conf文件
echo ""
echo "----->>>---->>>  mount 动态配置信息: "
mount -l
echo ""
echo "----->>>---->>>  selinux 动态配置信息: "
getsebool
sestatus
echo ""
echo "----->>>---->>>  建议禁用Transparent Huge Pages (THP): "
cat /sys/kernel/mm/transparent_hugepage/enabled
cat /sys/kernel/mm/transparent_hugepage/defrag
cat /sys/kernel/mm/redhat_transparent_hugepage/enabled
cat /sys/kernel/mm/redhat_transparent_hugepage/defrag
echo ""
echo "----->>>---->>>  硬盘SMART信息(需要root): "
smartctl --scan|awk -F "#" '{print $1}' | while read i; do echo -e "\n\nDEVICE $i"; smartctl -a $i; done
# 命令解析
# smartctl --scan : smartctl是一种磁盘自我分析检测技术
# 先列出设备, 然后遍历这些设备输出它们的详细信息
echo ""
echo "----->>>---->>>  当前用户的操作系统定时任务: "
echo "I am `whoami`"
crontab -l
echo "建议: "
echo "    仔细检查定时任务的必要性, 以及定时任务的成功与否的评判标准, 以及监控措施. "
echo "    请以启动数据库的OS用户执行本脚本. "
echo -e "\n"

# 下面开始输出系统日志内容
echo "----->>>---->>>  /var/log/boot.log "
cat /var/log/boot.log
echo ""
echo "----->>>---->>>  /var/log/cron(需要root) "
cat /var/log/cron
echo ""
echo "----->>>---->>>  /var/log/dmesg "
cat /var/log/dmesg
echo ""
echo "----->>>---->>>  /var/log/messages(需要root) "
tail -n 500 /var/log/messages
echo ""
echo "----->>>---->>>  /var/log/secure(需要root) "
cat /var/log/secure
echo ""
echo "----->>>---->>>  /var/log/wtmp "
who -a /var/log/wtmp
echo -e "\n"
