import * as std from 'std';
import * as os from 'os';

import engine from './template_engine.js';
import gp from './gpdb/greenplum.js';

import cpu from './linux/cpuinfo.js'
import disk from './linux/diskinfo.js'
import mem from './linux/meminfo.js'
import net from './linux/netinfo.js'

engine.options.debug = true;
engine.options.keepFormat = true;

export default function handle_api_request(request, response){

    // console.log(get_current_time(), "[REQ_START]", JSON.stringify(request));
    let start_time = new Date();

    try{

        switch(request.path){
        case '/1':
            console.log('parameter','a', '=', getParameter(request.query_string, 'a'));
            render_template(request, response);
            break;
        case '/2':
            pg_get_activity(request, response);
            break;
        case '/3':
            pg_get_settings(request, response);
            break;
        case '/4':
            pg_get_dashboard_stats(request, response);
            break;
        case '/gp/segments':
            gp_get_segment_configuration(request, response);
            break;
        case '/gp/activity_queries':
            gp_get_activity_query(request, response);
            break;
        case '/host/pref':
            host_get_pref_info(request, response);
            break;
        default:
            set_response_not_found(response);
        }


    }catch(err){
        response.status = 500;
        response.status_text = "Internal Server Error";
        fill_error_message(response, err);
    }finally{
        console.log(get_current_time(), "[REQ_ENDED]", JSON.stringify({
            stat:{
                used_time: new Date().getMilliseconds() - start_time.getMilliseconds()
            },
            request:{
                method: request.method,
                uri: request.uri
            },
            response:{
                status: response.status
            }
        }));
    }
};

function host_get_pref_info(request, response) {
    let start = new Date();
    let cpu_rates = cpu.get_cpu_used_rates();
    let mem_rate = mem.get_memory_used_rate();
    let disk_stats = disk.get_disk_io_stat("$1,$6,$7,$14");
    let net_stats = net.get_physical_network_interface_statistics();
    let end = new Date();

    let disks ={};
    for (let row = 0; row < disk_stats.body.length; row++) {
        const disk = disk_stats.body[row];
        disks[disk[0]] = {
            "read": parseFloat(disk[1]),  // read KB/s
            "write": parseFloat(disk[2]), // write KB/s
            "rate": parseFloat(disk[3]),  // %util
        };
    }


    let body = {
        cpu: cpu_rates,
        men: mem_rate,
        disk: disks,
        net: net_stats,
        start: start,
        end: end,
        used: end - start
    };
    set_response_ok(response, body);
}


function gp_get_segment_configuration(request, response){

    let client = gp.connect({
        // postgresql://[user[:password]@][netloc][:port][,...][/dbname][?param1=value1&...]
        url: 'postgresql://gpadmin:gpadmin@10.150.10.111:5432/postgres'
    });
    try{
        let options = {
            // format:'object'
        };
        let segments = client.get_segment_configuration(options);
        // console.log('segments', JSON.stringify(segments, undefined, 2));

        set_response_ok(response, segments);
    }finally{
        client.close();
    }

}

function gp_get_activity_query(request, response){

    do_query(function(client){
        let queries = client.list_activity_query();
        // console.log('segments', JSON.stringify(segments, undefined, 2));

        set_response_ok(response, queries);

    });

}

function do_query(callback, request, response){

    let client = gp.connect({
        // postgresql://[user[:password]@][netloc][:port][,...][/dbname][?param1=value1&...]
        url: 'postgresql://gpadmin:gpadmin@10.150.10.111:5432/postgres'
    });
    try{
        callback(client, request, response);
    }finally{
        client.close();
    }

}

function set_response_ok(response, body, status_message){
    response.status = 200;
    response.status_text = status_message || "OK";
    response.body = body == null ? null : JSON.stringify(body);
}
function set_response_not_found(response, body, status_message){
    response.status = 404;
    response.status_text = status_message || "Not Found";
    response.body = body == null ? null : JSON.stringify(body);
}

function get_current_time(){
    var d = new Date(), ms = d.getMilliseconds();
    return d.getHours()
    +":"+d.getMinutes()
    +":"+d.getSeconds()
    +"."+(ms>99?"":ms>9?"0":"00")+ms;
}

function getParameter(query_string, name) {
    var r = query_string.match(new RegExp("(^|&)" + name + "=([^&]*)(&|$)", "i"));
    return r == null ? null : unescape(r[2]);
}

function fill_error_message(response,err){
    if(err.stack){
        response.body = JSON.stringify({
            message: err.message,
            stack: err.stack.split('\n')
        });
    }else{
        response.body = JSON.stringify({
            message: err
        });
    }
}



function render_template(request, response){
    var template = '{{ d.name }}';
    var data = {
        name: 'abc'
    };
    var result = engine.template(template).render(data);

    response.status = 200;
    response.status_text = "OK";
    response.body = JSON.stringify({
        template: template,
        data: data,
        result : result
    });
}

// function render_template(request, response){

//     try{
//         var result = engine.template('{{ d.name }}').render({
//             name: 'abc'
//         });
        
//         response.status = 200;
//         response.status_text = "OK";
//         response.body = JSON.stringify(data);

//     }catch(err){
//         response.status = 400;
//         response.status_text = "Bad Request";
//         response.body = JSON.stringify(err);
//     }
// }

function pg_get_dashboard_stats(request, response){
    var template_file_path = 'templates/dashboard/sql/default/dashboard_stats.sql'
    ;
    query_postgresql_by_template(request, response, template_file_path);
}

function pg_get_settings(request, response){
    var template_file_path = 'templates/dashboard/sql/default/config.sql'
    ;
    query_postgresql_by_template(request, response, template_file_path);
}

function pg_get_activity(request, response){
    var template_file_path = 'templates/dashboard/sql/default/activity.sql'
    ;
    query_postgresql_by_template(request, response, template_file_path);
}

function query_postgresql_by_template(request, response, template_file_path, options){
    var template_content = engine.load(template_file_path)
    ;

    console.log("template_content", template_content);

    var tempalte = engine.template({
        //name: template_file_path,
        template: template_content,
    });
    var data = JSON.parse(request.body) || {};
    var sql = tempalte.render(data);
    console.log("sql", sql);

    query_postgresql(request, response, sql, options);
}


function query_postgresql(request, response, sql, options){
    try{
        var conn = pg_connect_db('');

        try{
            var rs = conn.query(sql);
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

            response.status = 200;
            response.status_text = "OK";
            response.body = JSON.stringify({
                // rows: rows,
                // cols: cols,
                head: head,
                data: data
            });

        }catch(err){
            throw err;
        }finally{
            conn.close();
        }

    }catch(err){
        response.status = 400;
        response.status_text = "Bad Request";
        fill_error_message(response, err);
    }
}