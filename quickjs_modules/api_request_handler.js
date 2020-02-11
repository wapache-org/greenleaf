function handle_api_request(request, response){

    console.log(JSON.stringify(request));

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

    console.log(JSON.stringify(response));
}