-- \echo Use "CREATE EXTENSION dbmaintain" to load this file. \quit

-- 标题: Greenplum表分区维护脚本
-- 版本: 19.10.28.152732
-- 说明: 
--       表: 默认指普通表, 分区父表 或 分区子表
--   分区表: 指分区父表
--   表分区: 指分区子表

-- ========================================================================================================================

-- show search_path;
-- create schema dbmaintain;
-- SET search_path TO dbmaintain,public;

-- \i dbmaintain.sql

-- SET search_path TO "$user",public, dbmaintain;
-- select create_table_partition('viid_motor_vehicle', '2020-03-01'::date, '2020-04-01'::date, 1, 'MONTH', '');

-- 分区备份序列, 用于生成备份文件的文件名
create sequence seq_backup_table_partition;

-- 分区维护表
-- insert into table_partition_maintain 
-- (id, table_name, partition_interval, maintain_type) values
-- ('CREATE_viid_motor_vehicle', 'viid_motor_vehicle', 'MONTH', 3);

create table table_partition_maintain (
    id varchar(255) primary key,      -- 主键列, 取值规则为 maintain_type+'_'+table_name
    table_name varchar(255),          -- 表名, 指分区父表
    partition_interval varchar(32),   -- 分区间隔: YEAR,MONTH,DAY, 暂时不支持时分秒和季度
    maintain_type varchar(32),        -- 维护类型: 创建分区 CREATE, 备份分区 BACKUP, 删除分区 DROP
    maintain_partition_count int,     -- 维护分区数量: 譬如 提前创建多少个分区, 备份多少个周期前的分区, 删除多少个周期前的分区
    create_with_options text,         -- 建表选项, 譬如 "with (append-only=true)"
    create_primary_key varchar(255),  -- 表主键列, 譬如id, 创建分区时Greenplum会自动处理主键, 所以此配置不起作用
    create_indexes text[],            -- 表索引, 格式: {'id,name','pass_time desc'}, 暂时不支持创建unique索引以及其他参数, 创建分区时Greenplum会自动处理主键, 所以此配置不起作用
    backup_location text default '/tmp',     -- 备份分区文件的路径, 不包含文件名, 当maintain_type='BACKUP' 或者 (maintain_type='DROP'且backup_before_drop=TRUE)时有效 
    backup_before_drop boolean default true, -- DROP表分区前是否备份表数据, 当maintain_type='DROP'时有效
    priority int default 0,           -- 执行优先级, 数值越小优先级越高, 不同类型之间相互独立, 创建(CREATE)优先于备份(BACKUP), 备份(BACKUP)优先于删除(DROP)。
    description text default ''       -- 说明
)
distributed by (id)
;

-- 分区维护执行日志表 
-- NOTE 目前没有用到, 只能通过查看数据库日志来查看执行情况。
create table table_partition_maintain_log 
(
    id SERIAL primary key,-- 自增主键列
    maintain_id int,      -- 维护信息的记录ID, table_partition_maintain.id
    execute_id text,      -- 执行维护的ID
    execute_sql text,     -- 执行的命令, 一次维护可能执行多个命令
    execute_log text,     -- 执行日志
    start_time timestamp, -- 开始执行命令的时间戳
    end_time timestamp,   -- 执行命令完成的时间戳
    is_success boolean    -- 是否执行成功
)
distributed by (id)
;

-- ========================================================================================================================

-- 通用函数: 是否Greenplum数据库
create or replace function is_greenplum()
returns boolean as $$ -- true: 是, false: 否
declare
    greenplum_position int; -- Greenplum 字样 在版本信息字符串中的位置
begin
    select position('Greenplum' in version()) into greenplum_position;
    return greenplum_position > 0;
end;
$$
language plpgsql;

-- 通用函数: 判断表是否存在
create or replace function table_exists
(
    table_name text -- 表名
)
returns boolean as $$ -- true: 存在, false: 不存在
begin
    if( (select 1 from pg_class where relname=table_name::name and relkind='r') is null  ) then
        return false;
    else
        return true;
    end if;
end;
$$
language plpgsql;

-- 通用函数: 判断表是否不存在
create or replace function table_not_exists
(
    table_name text -- 表名
)
returns boolean as $$ -- true: 不存在, false: 存在
begin
    if( (select 1 from pg_class where relname=table_name::name and relkind='r') is null  ) then
        return true;
    else
        return false;
    end if;
end;
$$
language plpgsql;

-- 通用函数: 判断表是否有默认分区
create or replace function table_has_default_partition
(
    table_name text -- 表名
)
returns boolean as $$ -- true: 有默认分区, false: 没有默认分区
begin
    if( (SELECT 1 FROM pg_partitions WHERE tablename = table_name and partitionIsDefault = true) is null  ) then
        return false;
    else
        return true;
    end if;
end;
$$
language plpgsql;

-- 通用函数: 判断表分区是否存在
create or replace function table_partition_exits
(
    table_name text, -- 表名
    partition_name text -- 分区名
)
returns boolean as $$ -- true: 存在, false: 不存在
begin
    if( (SELECT 1 FROM pg_partitions WHERE tablename = table_name and partitionname = partition_name) is null  ) then
        return false;
    else
        return true;
    end if;
end;
$$
language plpgsql;

-- 通用函数: 获取表分区的表名
create or replace function get_partition_table_name
(
    table_name text, -- 分区父表的表名
    partition_name text -- 分区名
)
returns text as $$ -- 分区子表的表名, 表分区不存在则返回NULL
declare
   partition_table_name text; -- 分区子表的表名
begin
    SELECT partitiontablename into partition_table_name FROM pg_partitions WHERE tablename = table_name and partitionname = partition_name;
    return partition_table_name;
end;
$$
language plpgsql;

-- 通用函数: 复制表数据到文件
-- NOTE: 没有测试过这个函数
create or replace function copy_table_to_file
(
    table_name varchar, -- 表名
    file_path text      -- 文件路径
)
returns void as $$
declare
    copy_sql text;      -- 复制表语句 
begin

    if file_path is null then
        raise exception 'copy table : %, file path required', table_name;
    end if;
    
    copy_sql :=
        'copy '||table_name ||
        ' to ''' || file_path || '/' || table_name || '-' || nextval('seq_backup_table_partition') || '.csv''' ||
        ' with HEADER CSV';
        
    raise notice 'copy table to file : %', copy_sql;
    execute copy_sql;
    
end;
$$
language plpgsql;


-- 通用函数: 备份分区
-- NOTE: 没有测试过这个函数
create or replace function backup_table_partition
(
    table_name varchar,     -- 表名称
    partition_name varchar, -- 分区名
    backup_path text        -- 备份文件的存放路径
)
returns void as $$
declare
    partition_table_name text; -- 分区子表名
    backup_sql text;           -- 备份表语句 
begin    
    select get_partition_table_name(table_name, partition_name) into partition_table_name;
    if partition_table_name is not null then
        raise notice 'backup table partition: % of %, partition table name is % ', partition_name, table_name, partition_table_name;
        perform copy_table_to_file(partition_table_name, backup_path);
    else
        raise notice 'backup table partition: % of %, partition not exist, skipping', partition_name, table_name;
    end if;
end;
$$
language plpgsql;

-- ========================================================================================================================

-- 私有函数: 创建分区
-- SET search_path TO "$user",public, dbmaintain;
-- select create_table_partition('viid_motor_vehicle', '2020-03-01'::date, '2020-04-01'::date, 1, 'MONTH', '');
create or replace function create_table_partition
(
    table_name text,    -- 分区父表名称
    start_date date,    -- 开始日期
    end_date date,      -- 结束日期
    interval_count int, -- 每多少个周期创建一个子分区表, 目前只支持1, 即分区子表只能按每天/每月/每年这种模式分区, 不能每7天分一个区
    interval_unit text, -- 周期单位, 取值范围参考table_partition_maintain.partition_interval
    with_options text   -- 建表选项
)
returns void as $$
declare
    table_name_date_part text;        -- 分区子表的表名中的日期部分
    partition_range_date_format text; -- 分区子表的分区条件的日期格式
    partition_name text;              -- 分区名
    add_partition_sql text;           -- 建表语句
    split_partition_sql text;         -- 拆分默认分区语句
BEGIN

    if interval_unit = 'DAY'::text then
        table_name_date_part := 'YYYYMMDD';
        partition_range_date_format := 'YYYY-MM-DD';
    elsif interval_unit = 'MONTH'::text then
        table_name_date_part := 'YYYYMM';
        partition_range_date_format := 'YYYY-MM-01';
    elsif interval_unit = 'YEAR' then
        table_name_date_part := 'YYYY';
        partition_range_date_format := 'YYYY-01-01';
    else
        raise exception 'create_table_partition: interval_unit''s value is not valid: %', interval_unit;
    end if;

    if table_has_default_partition(table_name) then
        for partition_name, split_partition_sql in 
            select
                'p' || to_char(d, table_name_date_part) as _partition_name
                ,
                'alter table ' || table_name
                || ' split default partition'
                || ' start (''' || to_char(d, partition_range_date_format) ||''') inclusive'
                || ' end(''' || to_char(d + (interval_count ||' '|| interval_unit)::interval, partition_range_date_format) ||''') exclusive'
                || ' into (partition p' || to_char(d, table_name_date_part) || ', partition pdefault)' as _split_partition_sql
            from generate_series(start_date, end_date, (interval_count ||' '||interval_unit)::interval) as d
        loop
            if( table_partition_exits(table_name, partition_name) ) then
                raise notice 'split default partition: % of %, partition already exists, skipping', partition_name, table_name;
            else
                raise notice 'split default partition: %', split_partition_sql;
                execute split_partition_sql;
            end if;
        end loop;
    else
        for partition_name, add_partition_sql in 
            select 
                'p' || to_char(d, table_name_date_part) as _partition_name
                ,
                'alter table ' || table_name
                || ' add partition p' || to_char(d, table_name_date_part)
                || ' start (''' || to_char(d, partition_range_date_format) ||''') inclusive'
                || ' end(''' || to_char(d + (interval_count ||' '|| interval_unit)::interval, partition_range_date_format) ||''') exclusive'
                || ' ' || with_options as _add_partition_sql
            from generate_series(start_date, end_date, (interval_count ||' '||interval_unit)::interval) as d
        loop
            if( table_partition_exits(table_name, partition_name) ) then
                raise notice 'create partition: % of %, partition already exists, skipping', partition_name, table_name;
            else
                raise notice 'create partition table: %', add_partition_sql;
                execute add_partition_sql;
            end if;
        end loop;
    end if;
end;
$$
language plpgsql;

-- 私有函数: 备份分区
-- NOTE: 没有测试过这个函数
create or replace function backup_table_partition
(
    table_name text,    -- 分区父表名称
    start_date date,    -- 开始日期
    end_date date,      -- 结束日期
    interval_count int, -- 每多少个周期创建一个子分区表, 目前只支持1, 即分区子表只能按每天/每月/每年这种模式分区, 不能每7天分一个区
    interval_unit text, -- 周期单位, 取值范围参考table_partition_maintain.partition_interval
    backup_path text    -- 备份文件的存放路径
)
returns void as $$
declare
    table_name_date_part text; -- 分区子表的表名中的日期部分  
    partition_name text;       -- 分区名
    backup_sql text;           -- 备份表语句 
begin

    if interval_unit = 'DAY'::text then
        table_name_date_part := 'YYYYMMDD';
    elsif interval_unit = 'MONTH'::text then
        table_name_date_part := 'YYYYMM';
    elsif interval_unit = 'YEAR' then
        table_name_date_part := 'YYYY';
    end if;
    
    for partition_name in 
        select 'p' || to_char(d, table_name_date_part) as _partition_name
        from generate_series(start_date, end_date, (interval_count||' '||interval_unit)::interval) as d
    loop
        perform backup_table_partition(table_name, partition_name, backup_path);
    end loop;

end;
$$
language plpgsql;

-- 私有函数, 删除表分区
create or replace function drop_table_partition
(
    table_name text,    -- 分区父表名称
    start_date date,    -- 开始日期
    end_date date,      -- 结束日期
    interval_count int, -- 每多少个周期创建一个子分区表, 目前只支持1, 即分区子表只能按每天/每月/每年这种模式分区, 不能每7天分一个区
    interval_unit text, -- 周期单位, 取值范围参考table_partition_maintain.partition_interval
    backup_before_drop boolean, -- 删除分区前是否执行备份操作
    backup_path text    -- 备份文件的路径
)
returns void as $$
declare
    partition_name text;       -- 分区名
    partition_table_name text; -- 分区子表的表名
    table_name_date_part text; -- 分区子表的表名中的日期部分
    drop_sql text;             -- 删表语句
begin

    if interval_unit = 'DAY'::text then
        table_name_date_part := 'YYYYMMDD';
    elsif interval_unit = 'MONTH'::text then
        table_name_date_part := 'YYYYMM';
    elsif interval_unit = 'YEAR' then
        table_name_date_part := 'YYYY';
    end if;
    
    for partition_name, drop_sql in 
        select
        'p' || to_char(d, table_name_date_part) as _partition_name
        , 
        'alter table '||table_name||' drop partition if exists ' || 'p' || to_char(d, table_name_date_part) as _drop_sql
        from generate_series(start_date, end_date, (interval_count||' '||interval_unit)::interval) as d
    loop
        if table_partition_exits(table_name, partition_name) then
            if backup_before_drop then
                perform backup_table_partition(table_name, partition_name, backup_path);
            end if;
            raise notice 'drop table partition: %', drop_sql;
            execute drop_sql;
        else 
            raise notice 'drop table partition: % of %, partition not exist, skipping', partition_name, table_name;
        end if;
    end loop;
end;
$$
language plpgsql;

-- ========================================================================================================================

-- 公共函数: 读取配置, 创建分区
create or replace function create_table_partition()
returns void as $$
declare
    now_time timestamp := now(); -- 当前时间
    start_date date;             -- 开始日期
    end_date date;               -- 结束日期
    conf table_partition_maintain%ROWTYPE; -- 配置信息
begin
    for conf in
        select * from table_partition_maintain where maintain_type = 'CREATE' order by priority asc
    loop
        start_date := now_time::date;
        end_date := ( now_time + ((conf.maintain_partition_count-1) ||' '||conf.partition_interval)::interval )::date;
        perform create_table_partition(
            conf.table_name, 
            start_date, end_date, 
            1, conf.partition_interval, 
            conf.create_with_options
        );
    end loop;
end;
$$
language plpgsql;

-- 公共函数: 读取配置, 备份分区
create or replace function backup_table_partition()
returns void as $$
declare
    now_time date := now(); -- 当前时间
    start_date date;        -- 开始日期
    end_date date;            -- 结束日期
    conf table_partition_maintain%ROWTYPE; -- 配置信息
begin
    for conf in
        select * from table_partition_maintain where maintain_type = 'BACKUP' order by priority asc
    loop
        start_date := (now_time - ((conf.maintain_partition_count-1) ||' '||conf.partition_interval)::interval)::date;
        end_date   := (now_time - (conf.maintain_partition_count     ||' '||conf.partition_interval)::interval)::date;

        perform backup_table_partition(
            conf.table_name, 
            start_date, end_date, 
            1, conf.partition_interval, 
            conf.maintain_partition_count, 
            conf.backup_location
        );
    end loop;
end;
$$
language plpgsql;

-- 公共函数: 读取配置, 删除分区
create or replace function drop_table_partition()
returns void as $$
declare
    now_time date := now(); -- 当前时间
    start_date date;        -- 开始日期
    end_date date;            -- 结束日期
    conf table_partition_maintain%ROWTYPE; -- 配置信息
begin
    for conf in
        select * from table_partition_maintain where maintain_type = 'DROP' order by priority asc
    loop
        start_date := (now_time - ((conf.maintain_partition_count+3) ||' '||conf.partition_interval)::interval)::date;
        end_date   := (now_time - ((conf.maintain_partition_count+1) ||' '||conf.partition_interval)::interval)::date;

        perform drop_table_partition(
            conf.table_name, 
            start_date, end_date, 
            1, conf.partition_interval,
            conf.backup_before_drop, 
            conf.backup_location
        );
    end loop;
end;
$$
language plpgsql;

-- ========================================================================================================================

-- 主入口函数, 读取配置, 维护表分区
create or replace function maintain_table_partition()
returns void as $$
BEGIN
  perform create_table_partition();
  perform backup_table_partition();
  perform drop_table_partition();
end;
$$
language plpgsql;


