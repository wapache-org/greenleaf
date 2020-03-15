import utils from './utils.js';

// 通过各种命令获取磁盘信息
var key_value_split_regexp = /\s*:\s*/,
    empty_line_regexp = /^\s*$/,
    column_split_regexp = /\s+/,
    key_value_split_regexp2 = /\s*=\s*/
;

function get_disk_io_stat(cols) {
    let cmd = "iostat -x | tail -n +6"+ (cols ? (" | awk '{ if($0!=\"\") { print "+cols+" } }'") : "");
    var info = utils.execute_cmd_to_table(cmd, column_split_regexp, true);
    // console.log(JSON.stringify(info, undefined, 2));
    return info;
}

function get_disk_used_rate() {
    var info = utils.execute_cmd_to_map("iostat -x | tail -n +7 | awk '{ if($0!=\"\") { print $1,$14 } }'", column_split_regexp);
    return info;
}

/**
 * 
 * @param {Object} options 
 * ```json
 * {
 *   "format":"map: 返回`Map<name, {}>`, 默认:返回数组",
 *   "filesystem":"boolean, 是否包含文件系统信息"
 * }
 * ```
 */
function get_disk_info(options){
    options = options || {};

    let info = {};
    let blockdevices = JSON.parse(utils.execute_cmd('lsblk -J -b -O -p')).blockdevices || [];
    if(options.filesystem){
        let fs = get_file_system_info();
        for (let i = 0; i < blockdevices.length; i++) {
            const disk = blockdevices[i];
            if(disk.mountpoint && fs[disk.mountpoint]){
                disk.filesystem = fs[disk.mountpoint];
            }else if(disk.children){
                disk.children.forEach(p=>{
                    if(p.mountpoint && fs[p.mountpoint]){
                        p.filesystem = fs[p.mountpoint];
                    }
                });
            }
        }
    }
    if(options.format==='map'){
        for (let i = 0; i < blockdevices.length; i++) {
            const disk = blockdevices[i];
            info[disk.name] = disk;
        }
    }else{
        info = blockdevices;
    }
    return info;
}

/**
 * 
 * @param {Object} options 
 * ```json
 * {
 *   
 *   "format": "table: 返回二维数组, flat: 返回数组, 数组的元素是json object, flat_map: 返回Map<mountpoint, object>, map:默认格式"
 * }
 * ```
 */
function get_file_system_info(options){
    options = options||{};

    let table = utils.execute_cmd_to_table("df -l -T", column_split_regexp, true), body = table.body;
    if(options.format === 'table'){
        return table;
    }

    let info = {}, partitions=[], filesytems=[];
    for (let i = 0; i < body.length; i++) {
        const row = body[i];
        // 文件系统       类型         1K-块     已用      可用 已用% 挂载点
        let fs = {
            part: row[0].startsWith('/dev/'), // 是否磁盘分区
            name: row[0],
            type: row[1],
            size: parseInt(row[2]),
            used: parseInt(row[3]),
            free: parseInt(row[4]),
            rate: parseFloat(row[5].substr(0,row[5].length-1)),
            mount: row[6]
        };
        if(fs.part){
            partitions.push(fs);
        }else{
            filesytems.push(fs);
        }
    }

    if(options.format === 'flat'){
        partitions.push(...filesytems);
        return partitions;
    } else if(options.format === 'flat_map'){
        partitions.forEach(fs => {
            info[fs.mount] = fs;
        });
        filesytems.forEach(fs => {
            info[fs.mount] = fs;
        });
        return info;
    }

    // 找/dev开头的, 以它的mont point 长度排序, 
    // 遍历其他非/dev开头的, 如果mount point 以第一步的mount point开头, 那么就是它下面的了

    partitions.sort(function(a,b){
        return b.name.length - a.name.length;
    });

    for (let i = 0; i < filesytems.length; i++) {
        const fs = filesytems[i];
        for (let j = 0; j < partitions.length; j++) {
            const p = partitions[j];
            if(fs.mount.startsWith(p.mount)){
                if(p.children === undefined) p.children = [];
                p.children.push(fs);
            }
        }
    }

    for (let j = 0; j < partitions.length; j++) {
        const p = partitions[j];
        info[p.mount] = p;
    }

    return info;
}


export default {
    get_disk_info,
    get_file_system_info,

    get_disk_io_stat,
    get_disk_used_rate
}

// /proc/diskstats




// iostat -x | tail -n +6


// lsblk
// 以json格式输出所有列, -p:输出全名, -b:单位是字节
// lsblk -J -b -O -p

// badblocks


// hdparm

// fdisk -l

// iotop

// iostat


// sar -b
// sar -d

// iotop


// pidstat


// lshw -class disk -short
