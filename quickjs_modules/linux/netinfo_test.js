import net from './netinfo.js'
import * as os from 'os'

var info = net.get_network_info();

//console.log(JSON.stringify(info, undefined, 2));
let netif = info.network_interface;
//console.log(JSON.stringify(netif.ipv4, undefined, 2));

// 收集3秒钟的统计信息
var stats = {};
for(let k in netif.ipv4){
    if(k.startsWith("en")){
        stats[k]=[];
        let stat = net.get_network_interface_statistics(k);
        stats[k].push(stat);
        // console.log(k, JSON.stringify(stat, undefined, 2));

        os.sleep(1000);
        stat = net.get_network_interface_statistics(k);
        stats[k].push(stat);

        os.sleep(1000);
        stat = net.get_network_interface_statistics(k);
        stats[k].push(stat);
    }
}

//console.log(JSON.stringify(stats, undefined, 2));

// 计算出网络流量
for(let k in stats){
    var d = stats[k];
    var count=0;
    let avg_rx_bytes=0, avg_tx_bytes=0;
    let last_rx_bytes=null, last_tx_bytes=null;
    for(let i=0;i<d.length;i++){
        let rx_bytes = parseInt(d[i]['rx_bytes']),
            tx_bytes = parseInt(d[i]['tx_bytes'])
        ;
        if(last_rx_bytes != null){
            count++;
            avg_rx_bytes += (rx_bytes - last_rx_bytes);
            avg_tx_bytes += (tx_bytes - last_tx_bytes);
        }
        last_rx_bytes = rx_bytes;
        last_tx_bytes = tx_bytes;
    }
    avg_rx_bytes = avg_rx_bytes / count;
    avg_tx_bytes = avg_tx_bytes / count;

    console.log(k, ': avg_rx_bytes=', avg_rx_bytes, ', avg_tx_bytes=', avg_tx_bytes);
}




// "collisions": "0",
// "multicast": "881787",
// "rx_bytes": "20603484485",
// "rx_compressed": "0",
// "rx_crc_errors": "0",
// "rx_dropped": "0",
// "rx_errors": "0",
// "rx_fifo_errors": "0",
// "rx_frame_errors": "0",
// "rx_length_errors": "0",
// "rx_missed_errors": "0",
// "rx_nohandler": "0",
// "rx_over_errors": "0",
// "rx_packets": "206741797",
// "tx_aborted_errors": "0",
// "tx_bytes": "399243091653",
// "tx_carrier_errors": "0",
// "tx_compressed": "0",
// "tx_dropped": "0",
// "tx_errors": "0",
// "tx_fifo_errors": "0",
// "tx_heartbeat_errors": "0",
// "tx_packets": "304844827",
// "tx_window_errors": "0"



