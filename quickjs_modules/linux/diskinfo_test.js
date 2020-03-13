import disk from './diskinfo.js'

var info = disk.get_disk_used_rate();
console.log(JSON.stringify(info, undefined, 2));

// var rate = mem.get_memory_used_rate();
// console.log('memory_used_rate:', rate);


