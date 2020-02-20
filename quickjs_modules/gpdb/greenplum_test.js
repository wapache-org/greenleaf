import gp from './greenplum.js'

let client = gp.connect({
    // postgresql://[user[:password]@][netloc][:port][,...][/dbname][?param1=value1&...]
    url: 'postgresql://gpadmin:gpadmin@10.150.10.111:5432/postgres'
});
try{
    // let version = client.get_version();
    // console.log('version', JSON.stringify(version, undefined, 2));

    // let dbs = client.list_database(undefined, true);
    // console.log('database list', JSON.stringify(dbs, undefined, 2));

    let roles = client.list_role(undefined, true);
    console.log('role list', JSON.stringify(roles, undefined, 2));

}finally{
    client.close();
}
