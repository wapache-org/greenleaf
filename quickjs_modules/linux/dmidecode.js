import utils from './utils.js';
/*
  bios
  system
  baseboard
  chassis
  processor
  memory
  cache
  connector
  slot
*/

let re  = /:\s*/,
    cmd = "dmidecode -q 2>/dev/null" // 需要root权限才能读取
;
var info = utils.execute_cmd_to_map2(cmd, null,null, '\t', re);

console.log(JSON.stringify(info, undefined, 2));

export default info;
