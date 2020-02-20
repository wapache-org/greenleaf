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
    query: function (sql, options){
        var rs = this.conn.query(sql);
        // rs.print();

        var rows = rs.getRowCount();
        var cols = rs.getColumnCount();
        //console.log('rows='+rows+', cols='+cols+'\n\n');

        var head = [];
        for(let col=0;col<cols;col++){
            head.push(rs.getColumnName(col));
        }

        var data = [];

        if(options && options.format=='object'){        
            for(let row=0;row<rows;row++){
                var r = {};
                for(let col=0;col<cols;col++){
                    r[head[col]]=rs.getValue(row, col);
                }
                data.push(r);
            }
        }else{
            for(let row=0;row<rows;row++){
                var r = [];
                for(let col=0;col<cols;col++){
                    r.push(rs.getValue(row, col));
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
    get_pset: function(){
        return {
            sversion: this.basedon.dbversion,
            dbversion: this.dbversion,
            dbtype: this.dbtype
        };
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
    }
};

var gp = {
    options: {
        url: '',
        cache_template: true
    },
    sqls: {
        select_version: 'select version();'
    },
    templates: {
        ccc: 'templates/psql/list_database.sql',
        list_role: 'templates/psql/list_role.sql'
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
