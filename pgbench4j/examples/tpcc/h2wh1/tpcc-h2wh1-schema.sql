
-- 删除表

drop table if exists warehouse;

drop table if exists item;

drop table if exists stock;

drop table if exists district;

drop table if exists customer;

drop table if exists oorder;

drop table if exists order_line;

drop table if exists history;

drop sequence if exists hist_id_seq;

drop table if exists new_order;

-- 创建表

-- 仓库
create table warehouse (
  w_id        integer   not null, -- 仓库ID
  w_ytd       decimal(12,2),      -- 
  w_tax       decimal(4,4),       -- 
  w_name      varchar(10),        -- 名称
  w_street_1  varchar(20),        -- 街道1
  w_street_2  varchar(20),        -- 街道2
  w_city      varchar(20),        -- 市
  w_state     char(2),            -- 省(州)
  w_zip       char(9)             -- 邮编
);

-- 区域, 一个仓库供应N个区域
create table district (
  d_w_id       integer       not null, -- 仓库ID
  d_id         integer       not null, -- 区域ID
  d_ytd        decimal(12,2),          --
  d_tax        decimal(4,4),           --
  d_next_o_id  integer,                --
  d_name       varchar(10),            -- 名称
  d_street_1   varchar(20),            -- 街道1
  d_street_2   varchar(20),            -- 街道2
  d_city       varchar(20),            -- 市
  d_state      char(2),                -- 省(州)
  d_zip        char(9)                 -- 邮编
);

-- 客户,
create table customer (
  c_w_id         integer        not null, -- 仓库ID
  c_d_id         integer        not null, -- 区域ID
  c_id           integer        not null, -- 客户ID
  c_discount     decimal(4,4),            --
  c_credit       char(2),                 --
  c_last         varchar(16),             -- 姓
  c_first        varchar(16),             -- 名
  c_credit_lim   decimal(12,2),           --
  c_balance      decimal(12,2),           --
  c_ytd_payment  float,                   --
  c_payment_cnt  integer,                 --
  c_delivery_cnt integer,                 --
  c_street_1     varchar(20),             -- 街道1
  c_street_2     varchar(20),             -- 街道2
  c_city         varchar(20),             -- 市
  c_state        char(2),                 -- 省(州)
  c_zip          char(9),                 -- 邮编
  c_phone        char(16),                --
  c_since        timestamp,               --
  c_middle       char(2),                 --
  c_data         varchar(500)             --
);

create sequence hist_id_seq;

create table history (
  hist_id  integer,
  h_c_id   integer,
  h_c_d_id integer,
  h_c_w_id integer,
  h_d_id   integer,
  h_w_id   integer,
  h_date   timestamp,
  h_amount decimal(6,2),
  h_data   varchar(24)
);

-- 订单
create table oorder (
  o_w_id       integer      not null, -- 仓库ID
  o_d_id       integer      not null, -- 区域ID
  o_id         integer      not null, -- 订单ID
  o_c_id       integer,               -- 客户ID
  o_carrier_id integer,
  o_ol_cnt     decimal(2,0),
  o_all_local  decimal(1,0),
  o_entry_d    timestamp
);

-- 新订单
create table new_order (
  no_w_id  integer   not null, -- 仓库ID
  no_d_id  integer   not null, -- 区域ID
  no_o_id  integer   not null  -- 订单ID
);

-- 订单明细
create table order_line (
  ol_w_id         integer   not null, -- 仓库ID
  ol_d_id         integer   not null, -- 区域ID
  ol_o_id         integer   not null, -- 订单ID
  ol_number       integer   not null, -- 订单项目序号
  ol_i_id         integer   not null, -- 订单项目ID
  ol_delivery_d   timestamp,
  ol_amount       decimal(6,2),
  ol_supply_w_id  integer,
  ol_quantity     decimal(2,0),
  ol_dist_info    char(24)
);

-- 库存
create table stock (
  s_w_id       integer       not null, -- 仓库ID
  s_i_id       integer       not null, -- 商品ID
  s_quantity   decimal(4,0),
  s_ytd        decimal(8,2),
  s_order_cnt  integer,
  s_remote_cnt integer,
  s_data       varchar(50),
  s_dist_01    char(24),
  s_dist_02    char(24),
  s_dist_03    char(24),
  s_dist_04    char(24),
  s_dist_05    char(24),
  s_dist_06    char(24),
  s_dist_07    char(24),
  s_dist_08    char(24),
  s_dist_09    char(24),
  s_dist_10    char(24)
);

-- 商品
create table item (
  i_id     integer      not null, -- 商品ID
  i_name   varchar(24),           -- 名称
  i_price  decimal(5,2),          -- 价格
  i_data   varchar(50),           --
  i_im_id  integer                --
);

-- 创建索引

alter table warehouse add constraint pk_warehouse
  primary key (w_id);

alter table district add constraint pk_district
  primary key (d_w_id, d_id);

alter table customer add constraint pk_customer
  primary key (c_w_id, c_d_id, c_id);

create index ndx_customer_name
  on  customer (c_w_id, c_d_id, c_last, c_first);

alter table oorder add constraint pk_oorder
  primary key (o_w_id, o_d_id, o_id);

create unique index ndx_oorder_carrier
  on  oorder (o_w_id, o_d_id, o_carrier_id, o_id);

alter table new_order add constraint pk_new_order
  primary key (no_w_id, no_d_id, no_o_id);

alter table order_line add constraint pk_order_line
  primary key (ol_w_id, ol_d_id, ol_o_id, ol_number);

alter table stock add constraint pk_stock
  primary key (s_w_id, s_i_id);

alter table item add constraint pk_item
  primary key (i_id);
