import utils from './utils.js';

let re  = /\s+/,
    cmd = "lsblk 2>/dev/null" // 有些参数需要root权限才能读取
;
var info = utils.execute_cmd_to_table(re, cmd);

console.log(JSON.stringify(info, undefined, 2));

export default info;
