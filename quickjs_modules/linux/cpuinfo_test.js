import cpu from './cpuinfo.js'

var info = cpu.get_cpu_info();

console.log(JSON.stringify(info, undefined, 2));
