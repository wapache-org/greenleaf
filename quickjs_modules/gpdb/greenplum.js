// import * as std from 'std';
// import * as os from 'os';
import engine from '../template_engine.js';

engine.options.debug = true;
engine.options.keepFormat = true;

var Client = function(options){
    this.options = options;
};
Client.prototype = {
    conn: null,
    sversion: 80400,
    connect: function(){
        if(this.conn) return;
        this.conn = pg_connect_db(this.options.url || '');
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
    list_database: function(pattern, verbose){
        var data = {
            verbose: verbose,
            pattern: pattern,
            pset: {
                sversion: this.sversion
            }
        }
        ,sql = gp.get_list_database_sql(data)
        ,res = this.query(sql);
        ;
        return res;
    }
};

var gp = {
    options: {
        url: '',
        cache_template: true,
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
    get_list_database_sql: function(data){
        var template_file_path = 'templates/psql/list_database.sql'
        ;
        return this.render(template_file_path, data);
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
        console.log("sql", sql);
        return sql;
    }

};

export default gp;
