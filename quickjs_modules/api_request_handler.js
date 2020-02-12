import engine from './template_engine.js';

export default function handle_api_request(request, response){

    console.log(JSON.stringify(request));

    try{
        console.log('parameter','a', '=', getParameter(request.query_string, 'a'));

        render_template(request, response);

    }catch(err){
        response.status = 500;
        response.status_text = "Internal Server Error";
        response.body = JSON.stringify(err);
    }finally{
        console.log(JSON.stringify({
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

function getParameter(query_string, name) {
    var r = query_string.match(new RegExp("(^|&)" + name + "=([^&]*)(&|$)", "i"));
    return r == null ? null : unescape(r[2]);
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

function pg_get_settings(request, response){
    try{
        var conn = pg_connect_db('');

        try{
            var rs = conn.query('select * from pg_settings');
            // rs.print();

            var rows = rs.getRowCount();
            var cols = rs.getColumnCount();
            console.log('rows='+rows+', cols='+cols+'\n\n');
            var data = [];
            for(row=0;row<rows;row++){
                var r = {};
                for(col=0;col<cols;col++){
                    r[rs.getColumnName(col)]=rs.getValue(row, col);
                }
                data.push(r);
            }
            rs.close();

            response.status = 200;
            response.status_text = "OK";
            response.body = JSON.stringify(data);

        }catch(err){
            throw err;
        }finally{
            conn.close();
        }

    }catch(err){
        response.status = 400;
        response.status_text = "Bad Request";
        response.body = JSON.stringify(err);
    }
}
