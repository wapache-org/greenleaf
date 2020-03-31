import cpu from './cpuinfo.js'
import mem from './meminfo.js'
import disk from './diskinfo.js'
import net from './netinfo.js'

var rs;

let start = new Date();

var cpu_detail_rate = cpu.get_cpu_used_rates();
let cpu_rate = {"cpu": cpu_detail_rate.cpu};
// console.log('cpu_detail_rate', JSON.stringify(cpu_detail_rate, undefined, 2));

var mem_rate = mem.get_memory_used_rate();
// console.log('mem_rate:', JSON.stringify(mem_rate, undefined, 2));


let disk_stat = disk.get_disk_io_stat("$1,$6,$7,$14");
// console.log('disk_rate', JSON.stringify(disk_stat, undefined, 2));

let disk_rate ={};
for (let row = 0; row < disk_stat.body.length; row++) {
    const disk = disk_stat.body[row];
    disk_rate[disk[0]] = {
        "read": parseFloat(disk[1])*1024,  // B/s
        "write": parseFloat(disk[2])*1024, // B/s
        "rate": parseFloat(disk[3]),  // %util
    };
}
// console.log('disk_rate', JSON.stringify(disk_rate, undefined, 2));


// let info = net.get_physical_network_interface_statistics();
// console.log('net_rate', JSON.stringify(info, undefined, 2));


let net_rate = sys_get_pni_speed();
// console.log('net_rate', JSON.stringify(net_rate, undefined, 2));

let end = new Date();

// 一天一个表, 假设每秒钟采集一次, 每个表记录数: 3600*24
let table_name = "metrics_host_"+date_formatter(start);

rs = sqlite_exec(`
CREATE TABLE IF NOT EXISTS ${table_name}(
    id int8 PRIMARY KEY, 
    start_time timestamp, 
    end_time timestamp, 
    cpu text, 
    cpu_detail text, 
    memory text, 
    disk text, 
    network text
);`
);
if(rs.code) console.log("CREATE", JSON.stringify(rs, undefined, 2));

rs = sqlite_exec2(`INSERT OR REPLACE INTO ${table_name} VALUES (?, ?, ?, ?, ?, ?, ?, ?);`, 
    Math.floor(start.getTime()/1000), 
    start.getTime(), 
    end.getTime(), 
    JSON.stringify(cpu_rate), 
    JSON.stringify(cpu_detail_rate), 
    JSON.stringify(mem_rate), 
    JSON.stringify(disk_rate), 
    JSON.stringify(net_rate)
);
if(rs.code) console.log("INSERT", JSON.stringify(rs, undefined, 2));

let query = `
SELECT id, datetime(id, 'unixepoch') as collect_time, end_time - start_time as duration, cpu, memory, disk, network
FROM metrics_host_20200331
order by id desc;
`;

/**
  * yyyyMMdd
 */
function date_formatter (date) {
    const month = (date.getMonth() + 1).toString().padStart(2, '0');
    const strDate = date.getDate().toString().padStart(2, '0');
    return `${date.getFullYear()}${month}${strDate}`;
}