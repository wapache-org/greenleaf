import disk from './diskinfo.js'

// test1();
// test3();
test_get_disk_info();

function test1(){

    let disk_stats = disk.get_disk_io_stat("$1,$6,$7,$14");
    // console.log(JSON.stringify(disk_stats, undefined, 2));

    let disks ={};
    for (let row = 0; row < disk_stats.body.length; row++) {
        const disk = disk_stats.body[row];
        disks[disk[0]] = {
            "read": parseFloat(disk[1]),  // read KB/s
            "write": parseFloat(disk[2]), // write KB/s
            "rate": parseFloat(disk[3]),  // %util
        };
    }
    console.log(JSON.stringify(disks, undefined, 2));

}

function test2(){
    var info = disk.get_disk_used_rate();
    console.log(JSON.stringify(info, undefined, 2));
}


function test3(){
    console.log('default', JSON.stringify(disk.get_file_system_info(), undefined, 2));
    console.log('flat', JSON.stringify(disk.get_file_system_info({format:'flat'}), undefined, 2));
    console.log('table', JSON.stringify(disk.get_file_system_info({format:'table'}), undefined, 2));
}


function test_get_disk_info(){
    console.log('get_disk_info', JSON.stringify(disk.get_disk_info({filesystem: true}), undefined, 2));
}