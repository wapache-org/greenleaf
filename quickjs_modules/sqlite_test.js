
var rs;

rs = sqlite_exec("CREATE TABLE IF NOT EXISTS kv(key PRIMARY KEY, val);");
console.log("CREATE", JSON.stringify(rs, undefined, 2));

rs = sqlite_exec("INSERT OR REPLACE INTO kv VALUES (1, 'a');");
console.log("INSERT", JSON.stringify(rs, undefined, 2));

rs = sqlite_exec("select * from kv;");
console.log("SELECT", JSON.stringify(rs, undefined, 2));
