import mem from './meminfo.js'

var info = mem.get_memory_info();
//console.log(JSON.stringify(info, undefined, 2));

var rate = mem.get_memory_used_rate();
console.log('memory_used_rate:', rate);


