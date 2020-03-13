import cpu from './cpuinfo.js'

// var info = cpu.get_cpu_info();
//console.log(JSON.stringify(info, undefined, 2));


// var rate = cpu.get_cpu_total_used_rate();
// console.log('cpu_used_rate:', rate, '%');


var rates = cpu.get_cpu_used_rates();
console.log(JSON.stringify(rates, undefined, 2));

