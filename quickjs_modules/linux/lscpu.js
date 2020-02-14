import * as std from 'std';

var info = {};

let line, 
    re  = /\s*:\s*/,
    cmd = "/usr/bin/lscpu 2>/dev/null"
;
let pipe = std.popen(cmd, "r");
while ((line = pipe.getline()) != null) {
    // console.log('line', line);
    if(line.length==0){
        continue;
    }
    let part = line.split(re);
    info[part[0]] = part[1];
}
pipe.close();

console.log(JSON.stringify(info, undefined, 4));

export default info;

/*

Architecture:          x86_64
CPU op-mode(s):        32-bit, 64-bit
Byte Order:            Little Endian
CPU(s):                8
On-line CPU(s) list:   0-7
Thread(s) per core:    2
Core(s) per socket:    4
Socket(s):             1
NUMA node(s):          1
Vendor ID:             GenuineIntel
CPU family:            6
Model:                 42
Model name:            Intel(R) Core(TM) i7-2760QM CPU @ 2.40GHz
Stepping:              7
CPU MHz:               1418.389
CPU max MHz:           3500.0000
CPU min MHz:           800.0000
BogoMIPS:              4784.60
Virtualization:        VT-x
L1d cache:             32K
L1i cache:             32K
L2 cache:              256K
L3 cache:              6144K
NUMA node0 CPU(s):     0-7
Flags:                 fpu vme de

*/