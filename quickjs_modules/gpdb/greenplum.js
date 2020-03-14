// import * as std from 'std';
// import * as os from 'os';
import engine from '../template_engine.js';

engine.options.debug = false;
engine.options.keepFormat = false;


var Client = function(options){
    this.options = options;
};
Client.prototype = {
    conn: null,
    dbversion: 51700,
    dbtype: 'greenplum',
    basedon:{
        dbversion: 80323,
        dbtype: 'postgresql',
    },
    connect: function(){
        if(this.conn) return;
        this.conn = pg_connect_db(this.options.url || '');
        var v = this.get_version();

        this.dbversion = v.greenplum_num;
        this.basedon.dbversion = v.postgresql_num;
    },
    close: function(){
        if(this.conn){
            this.conn.close();
            this.conn = null;
        }
    },
    /**
     * 执行查询语句,并返回查询结果
     * @param {string} sql 查询语句
     * @param {object} options 查询选项
     * ```json
     * {
     * "format": "null: 以二维数组形式返回查询结果, object: 以array+key-value形式返回查询结果"
     * } 
     * ```
     */
    query: function (sql, options){
        var rs = this.conn.query(sql);
        // rs.print();

        var rows = rs.getRowCount();
        var cols = rs.getColumnCount();
        //console.log('rows='+rows+', cols='+cols+'\n\n');

        var head = [];
        var types = [];
        for(let col=0;col<cols;col++){
            head.push(rs.getColumnName(col));
            types.push(rs.getColumnType(col));
        }

        var data = [];

        if(options && options.format=='object'){
            for(let row=0;row<rows;row++){
                var r = {};
                for(let col=0;col<cols;col++){
                    r[head[col]]=gp.types.parse(types[col], rs.getValue(row, col));
                }
                data.push(r);
            }
        }else{
            for(let row=0;row<rows;row++){
                var r = [];
                for(let col=0;col<cols;col++){
                    r.push(gp.types.parse(types[col], rs.getValue(row, col)));
                }
                data.push(r);
            }
        }

        rs.close();

        return {
            head: head,
            data: data
        };
    },
    /**
     * 获取数据库版本信息
     */
    get_version: function(){
        var res = this.query(gp.sqls.select_version);
        var version = res.data[0][0];

        var reg = /PostgreSQL\s+(\d+\.\d+(?:\.\d+)?)\s+\(Greenplum\s+Database\s+(\d+\.\d+(?:\.\d+)?)/;
        var ver = reg.exec(version);

        var pg_ver = ver[1].trim();
        var gp_ver = ver[2].trim();

        var pg_vers = pg_ver.split('.');
        var gp_vers = gp_ver.split('.');
        var pg_ver_num = 0;
        var gp_ver_num = 0;
        for(let i=0;i<2;i++){
            pg_ver_num += parseInt(pg_vers[i] || 0) * (i===0?10000:i===1?100:1);
            gp_ver_num += parseInt(gp_vers[i] || 0) * (i===0?10000:i===1?100:1);
        }

        return {
            version: version,
            postgresql: pg_ver,
            greenplum : gp_ver,
            postgresql_num: pg_ver_num,
            greenplum_num : gp_ver_num
        };
    },
    /**
     * 获取数据库设置.
     * 
     * pset是模板内置变量, 此函数用于配合模板引擎工作.
     * 
     */
    get_pset: function(){
        return {
            sversion: this.basedon.dbversion,
            dbversion: this.dbversion,
            dbtype: this.dbtype
        };
    },
    get_segment_configuration: function(options){
        return this.query(gp.sqls.select_segment_configuration, options);
    },
    list_database: function(pattern, verbose){
        var data = {
            verbose: verbose,
            pattern: pattern,
            pset: this.get_pset()
        };
        // console.log(JSON.stringify((data)));
        var sql = gp.render(gp.templates.list_database, data)
        ,res = this.query(sql);
        ;
        return res;
    },
    list_role: function(pattern, verbose){
        var data = {
            verbose: verbose,
            pattern: pattern,
            pset: this.get_pset()
        };
        // console.log(JSON.stringify((data)));
        var sql = gp.render(gp.templates.list_role, data)
        ,res = this.query(sql);
        ;
        return res;
    },
    list_activity_query: function(){
        var data = {
            pset: this.get_pset()
        };
        // console.log(JSON.stringify((data)));
        var sql = gp.render(gp.templates.list_activity_query, data)
        ,res = this.query(sql);
        ;
        return res;
    }
};

var gp = {
    options: {
        url: '',
        cache_template: true
    },
    // 数据类型对应的oid值
    types:{
        bool: 16,
        bytea: 17, // byte array
        char: 18, 
        name: 19, // 63-byte type for storing system identifiers
        int8: 20,
        int2: 21,
        int2vector: 22, // array of int2, used in system tables
        int4: 23,
        regproc: 24, // registered procedure
        text: 25,
        oid: 26, // object identifier(oid), maximum 4 billion
        tid: 27, // (block, offset), physical location of tuple
        xid: 28, // transaction id
        cid: 29, // command identifier type, sequence in transaction id
        oidvector: 30, // array of oids, used in system tables

        json: 114,
        xml: 142,

        float4: 700,
        float8: 701,
        
        bpchar: 1042, // char(length), blank-padded string, fixed storage length
        varchar: 1043, // varchar(length), non-blank-padded string, variable storage length

        date: 1082,
        time: 1083,
        timestamp: 1114, // date and time
        timestamptz: 1184, // date and time with time zone
        interval: 1186, // @ <number> <units>, time interval
        timetz: 1266, // time of day with time zone

        bit: 1560, // fixed-length bit string
        varbit: 1562, // variable-length bit string

        numeric: 1700, // numeric(precision, decimal), arbitrary precision number
        uuid: 2950, // UUID datatype

        jsonb: 3802, // Binary JSON
        jsonpath: 4072, // JSON path

        is_integer: function(v){
            return v === this.int2 
            || v === this.int4
            || v === this.int8
            || v === this.oid
            || v === this.xid
            || v === this.cid
            ;
        },
        is_float: function(v){
            return v === this.float4 
            || v === this.float8
            || v === this.numeric
            ;
        },
        is_number: function(v){
            return this.is_integer(v) || this.is_float(v);
        },
        parse: function(type, value){
            if(value!==null && value !== undefined) {
                if(this.is_integer(type)){
                    value = parseInt(value);
                }else if(this.is_float(type)){
                    value = parseFloat(value);
                }else if(this.bool === type){
                    value = value == 't';
                }
            }
            return value;
        }

    },
    sqls: {
        select_version: 'select version()',
        select_segment_configuration:'select * from gp_segment_configuration',
        select_activity_query:'select * from pg_stat_activity'
    },
    templates: {
        list_database: 'templates/psql/list_database.sql',
        list_role: 'templates/psql/list_role.sql',
        list_activity_query: 'templates/gpdb/pg_stat_activity.sql'
    },
    connect: function(options){
        var opt = {};

        for(var p in options) opt[p]=options[p];

        for(var p in gp.options)
          if(opt[p]==undefined) opt[p]=gp.options[p];
  
        var client = new Client(opt);
        client.connect();
        return client;
    },
    render: function (template_file_path, data){
        let template_content = engine.load(template_file_path)
        ;
        // console.log("template_content", template_content);
        let tempalte = engine.template({
            name: this.options.cache_template ? template_file_path : undefined,
            template: template_content,
        });
        let sql = tempalte.render(data || {});
        //console.log("sql", sql);
        return sql;
    }

};

export default gp;
