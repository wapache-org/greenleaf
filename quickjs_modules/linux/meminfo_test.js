import mem from './meminfo.js'

var info = mem.get_memory_info();

console.log(JSON.stringify(info, undefined, 2));
