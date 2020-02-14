import * as std from 'std';

var info = {};

let line, 
    re  = /\s*:\s*/,
    cmd = "/usr/bin/lsb_release -idrc 2>/dev/null"
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

#if defined(SLES12) || defined(SLES15)
    pop = popen("cat /etc/os-release 2>/dev/null", "r");
#else
    pop = popen("cat /etc/*ease 2>/dev/null", "r");
#endif

cat /etc/os-release 2>/dev/null




*/