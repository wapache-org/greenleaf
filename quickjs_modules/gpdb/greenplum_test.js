import gp from './greenplum.js'

var client = gp.connect({
    // postgresql://[user[:password]@][netloc][:port][,...][/dbname][?param1=value1&...]
    url: 'postgresql://gpadmin:gpadmin@10.150.10.111:5432/postgres'
});

var dbs = client.list_database(undefined, true);

console.log(JSON.stringify(dbs, undefined, 2));
