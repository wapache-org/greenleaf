#!/bin/bash

# 已在CentOS 6.x上进行测试
# author: digoal
# 2015-10
# 权限需求 , OS: root PG: Superuser
# 用法  . ./generate_report.sh >/tmp/report.log 2>&1


# 生成报告目录   grep -E "^----->>>|^\|" /tmp/report.log | sed 's/^----->>>---->>>/    /' | sed '1 i\ \ 目录\n\n' | sed '$ a\ \n\n\ \ 正文\n\n'
# 命令详解:
# grep -E "^----->>>|^\|" /tmp/report.log 
#   作用: 从报告文件中过滤出目录行
#   过滤规则: ^----->>>|^\|
# sed 's/^----->>>---->>>/    /'
#   作用: 将"目录文本"前的"目录标记"替换成4个空格
#   sed是流编辑器, 单引号中的是sed的脚本命令
#   s命令的格式: [address]s/pattern/replacement/flags
#   可以看到: pattern就是^----->>>---->>>了, replacement是    , 没有address和flags
# sed '1 i\ \ 目录\n\n'
#   作用: 在第一行插入"目录"字样
#   i命令格式: [address]i\new_content
# sed '$ a\ \n\n\ \ 正文\n\n'
#   作用: 在最后一行插入"正文"字样

# 请将以下变量修改为与当前环境一致, 并且确保使用这个配置连接任何数据库都不需要输入密码
export PGHOST=127.0.0.1
export PGPORT=5432
export PGDATABASE=postgres
export PGUSER=postgres
export PGPASSWORD=postgres
export PGHOME=/opt/pgsql-12
export PGDATA=$PGHOME/data

export PATH=$PGHOME/bin:$PATH:.
export DATE=`date +"%Y%m%d%H%M"`
export LD_LIBRARY_PATH=$PGHOME/lib:/lib64:/usr/lib64:/usr/local/lib64:/lib:/usr/lib:/usr/local/lib:$LD_LIBRARY_PATH


# 记住当前目录
PWD=`pwd`

# 获取postgresql日志目录
pg_log_dir=`grep '^\ *[a-z]' $PGDATA/postgresql.conf|awk -F "#" '{print $1}'|grep log_directory|awk -F "=" '{print $2}'`
# 命令解析:
#   grep '^\ *[a-z]' $PGDATA/postgresql.conf
#     去掉文件中的注释, 只保留字母或空格开头的行
#   awk -F "#" '{print $1}'
#     -F指定分隔符为#号
#     {print $1} 输出用分隔符分割后的第一列
#   grep log_directory
#     过滤出日子目录配置项
#   awk -F "=" '{print $2}'
#     -F指定分隔符为=号
#     {print $2} 输出用分隔符分割后的第2列, 也就是日志所在的目录了

# 检查是否standby
is_standby=`psql --pset=pager=off -q -A -t -c 'select pg_is_in_recovery()'`


echo "    ----- PostgreSQL 巡检报告 -----  "
echo "    ===== $DATE        =====  "


echo "|+++++++++++++++++++++++++++++++++++++++++++++++++++++++++|"
echo "|                       数据库信息                        |"
echo "|+++++++++++++++++++++++++++++++++++++++++++++++++++++++++|"
echo ""

echo "----->>>---->>>  数据库版本: "
psql --pset=pager=off -q -c 'select version()'

echo "----->>>---->>>  用户已安装的插件版本: "
for db in `psql --pset=pager=off -t -A -q -c 'select datname from pg_database where datname not in ($$template0$$, $$template1$$)'`
do
psql -d $db --pset=pager=off -q -c 'select current_database(),* from pg_extension'
done
# 先查出数据库, 然后逐个查插件

echo "----->>>---->>>  用户使用了多少种数据类型: "
for db in `psql --pset=pager=off -t -A -q -c 'select datname from pg_database where datname not in ($$template0$$, $$template1$$)'`
do
psql -d $db --pset=pager=off -q -c 'select current_database(),b.typname,count(*) from pg_attribute a,pg_type b where a.atttypid=b.oid and a.attrelid in (select oid from pg_class where relnamespace not in (select oid from pg_namespace where nspname ~ $$^pg_$$ or nspname=$$information_schema$$)) group by 1,2 order by 3 desc'
done

echo "----->>>---->>>  用户创建了多少对象: "
for db in `psql --pset=pager=off -t -A -q -c 'select datname from pg_database where datname not in ($$template0$$, $$template1$$)'`
do
psql -d $db --pset=pager=off -q -c 'select current_database(),rolname,nspname,relkind,count(*) from pg_class a,pg_authid b,pg_namespace c where a.relnamespace=c.oid and a.relowner=b.oid and nspname !~ $$^pg_$$ and nspname<>$$information_schema$$ group by 1,2,3,4 order by 5 desc'
done

echo "----->>>---->>>  用户对象占用空间的柱状图: "
for db in `psql --pset=pager=off -t -A -q -c 'select datname from pg_database where datname not in ($$template0$$, $$template1$$)'`
do
psql -d $db --pset=pager=off -q -c 'select current_database(),buk this_buk_no,cnt rels_in_this_buk,pg_size_pretty(min) buk_min,pg_size_pretty(max) buk_max from( select row_number() over (partition by buk order by tsize),tsize,buk,min(tsize) over (partition by buk),max(tsize) over (partition by buk),count(*) over (partition by buk) cnt from ( select pg_relation_size(a.oid) tsize, width_bucket(pg_relation_size(a.oid),tmin-1,tmax+1,10) buk from (select min(pg_relation_size(a.oid)) tmin,max(pg_relation_size(a.oid)) tmax from pg_class a,pg_namespace c where a.relnamespace=c.oid and nspname !~ $$^pg_$$ and nspname<>$$information_schema$$) t, pg_class a,pg_namespace c where a.relnamespace=c.oid and nspname !~ $$^pg_$$ and nspname<>$$information_schema$$ ) t)t where row_number=1;'
done



if [ $is_standby == 't' ]; then
echo "    ===== 这是standby节点     =====  "
else
echo "    ===== 这是primary节点     =====  "
fi
echo ""


primary() {
echo "----->>>---->>>  获取recovery.done md5值: "
md5sum $PGDATA/recovery.done
echo "建议: "
echo "    主备md5值一致(判断主备配置文件是否内容一致的一种手段, 或者使用diff)."
echo -e "\n"

echo "----->>>---->>>  获取recovery.done配置: "
grep '^\ *[a-z]' $PGDATA/recovery.done|awk -F "#" '{print $1}'
echo "建议: "
echo "    在primary_conninfo中不要配置密码, 容易泄露. 建议为流复制用户创建replication角色的用户, 并且配置pg_hba.conf只允许需要的来源IP连接. "
echo -e "\n"
}  # primary function end

standby() {
echo "----->>>---->>>  获取recovery.conf md5值: "
md5sum $PGDATA/recovery.conf
echo "建议: "
echo "    主备md5值一致(判断主备配置文件是否内容一致的一种手段, 或者使用diff)."
echo -e "\n"

echo "----->>>---->>>  获取recovery.conf配置: "
grep '^\ *[a-z]' $PGDATA/recovery.conf|awk -F "#" '{print $1}'
echo "建议: "
echo "    在primary_conninfo中不要配置密码, 容易泄露. 建议为流复制用户创建replication角色的用户, 并且配置pg_hba.conf只允许需要的来源IP连接. "
echo -e "\n"
}  # standby function end


adds() {
echo "|+++++++++++++++++++++++++++++++++++++++++++++++++++++++++|"
echo "|                        附加信息                         |"
echo "|+++++++++++++++++++++++++++++++++++++++++++++++++++++++++|"
echo ""

echo "----->>>---->>>  附件1 : `date -d '-1 day' +"%Y-%m-%d"` 操作系统sysstat收集的统计信息 "
sar -A -f /var/log/sa/sa`date -d '-1 day' +%d`
echo -e "\n"

echo "----->>>---->>>  其他建议: "
echo "    其他建议的巡检项: "
echo "        HA 状态是否正常, 例如检查HA程序, 检查心跳表的延迟. "
echo "        sar io, load, ...... "
echo "    巡检结束后, 清理csv日志 "
}  # adds function end


if [ $is_standby == 't' ]; then
standby
else
primary
fi

common
adds
cd $pwd
return 0

#  备注
#  csv日志分析需要优化
#  某些操作需要root
