import * as std from 'std';

var info = {};

let line, 
    re  = /\s*:\s*/,
    cmd = "cat /proc/meminfo 2>/dev/null"
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
