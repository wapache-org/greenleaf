
//test();

test2();

function test() {
    var rs;
    
    rs = sqlite_exec("CREATE TABLE IF NOT EXISTS kv(key int PRIMARY KEY, val text);");
    console.log("CREATE", JSON.stringify(rs, undefined, 2));
    if(rs.code) return;
    
    // rs = sqlite_exec("select max(key) as max_key from kv;");
    // console.log("SELECT", JSON.stringify(rs, undefined, 2));
    // if(rs.code) return;
    
    // max_key = parseInt(rs.rows[0][0]);
    rs = sqlite_exec2("select max(key) as max_key from kv;");
    console.log("SELECT", JSON.stringify(rs, undefined, 2));
    if(rs.code) return;
    
    max_key = rs.rows[0][0];
    rs = sqlite_exec2("INSERT OR REPLACE INTO kv VALUES (?, ?);", max_key+1, 'a');
    console.log("INSERT", JSON.stringify(rs, undefined, 2));
    //if(rs.code) return;
    
    rs = sqlite_exec2("select * from kv;");
    console.log("SELECT", JSON.stringify(rs, undefined, 2));
    if(rs.code) return;
    
    rs = sqlite_exec2("delete from kv where key < ?", max_key);
    console.log("DELETE", JSON.stringify(rs, undefined, 2));
}

function test2()
{
    var rs;

    rs = sqlite_exec2("select * from metrics_linux_stat;");
    console.log("SELECT", JSON.stringify(rs, undefined, 2));

}