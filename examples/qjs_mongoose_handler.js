function handle(req, resp){
    console.log(JSON.stringify(req));

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

            resp.status = 200;
            resp.status_text = "OK";
            resp.body = JSON.stringify(data);

        }catch(err){
            throw err;
        }finally{
            conn.close();
        }

    }catch(err){
        resp.status = 400;
        resp.status_text = "Bad Request";
        resp.body = JSON.stringify(err);
    }

    console.log(JSON.stringify(resp));

}







