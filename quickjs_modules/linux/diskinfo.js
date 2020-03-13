import utils from './utils.js';

// 通过各种命令获取cpu信息
var key_value_split_regexp = /\s*:\s*/,
    empty_line_regexp = /^\s*$/,
    column_split_regexp = /\s+/,
    key_value_split_regexp2 = /\s*=\s*/
;

function get_disk_io_stat() {
    var raw_info = utils.execute_cmd_to_table("iostat -x | tail -n +6", column_split_regexp, true);
    var info = raw_info;

    // console.log(JSON.stringify(info, undefined, 2));
    return info;
}

function get_disk_used_rate() {
    var info = utils.execute_cmd_to_map("iostat -x | tail -n +7 | awk '{ if($0!=\"\") { print $1,$14 } }'", column_split_regexp);
    return info;
}

export default {

    get_disk_io_stat,
    get_disk_used_rate

}

// /proc/diskstats

// iostat -x | tail -n +6


// lsblk

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
