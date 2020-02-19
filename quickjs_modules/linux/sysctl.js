import utils from './utils.js';

let re  = /\s*=\s*/,
    cmd = "sysctl -a 2>/dev/null" // 有些参数需要root权限才能读取
;
var info = utils.execute_cmd_to_map(re, cmd);

console.log(JSON.stringify(info, undefined, 4));

export default info;
