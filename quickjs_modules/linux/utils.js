import * as std from 'std';

/**
 * 
 * @param {string} cmd 
 * @param {RegExp} key_value_spliter_re 
 * @returns {object} {partN:valueN}
 */
function execute_cmd_to_map(cmd, key_value_spliter_re) {

    var info = {};

    let line
    ;
    let pipe = std.popen(cmd, "r");
    while ((line = pipe.getline()) != null) {
        // console.log('line', line);
        if(line.length==0){
            continue;
        }
        let part = line.split(key_value_spliter_re);
        info[part[0].trim()] = part[1];
    }
    pipe.close();

    return info;

}

/**
 * 
 * @param {string} cmd 
 * @param {RegExp} key_value_spliter_re 
 * @returns {object} {partN:[value1,...,valueN]}
 */
function execute_cmd_to_map_array(cmd, key_value_spliter_re) {

    var info = {};

    let line
    ;
    let pipe = std.popen(cmd, "r");
    while ((line = pipe.getline()) != null) {
        // console.log('line', line);
        if(line.length==0){
            continue;
        }
        let part = line.split(key_value_spliter_re);
        if(info[part[0]] === undefined){
            info[part[0]] = [];
        }
        info[part[0]].push(part[1]);
    }
    pipe.close();

    return info;

}

/**
 * 用于解析类似 `dmidecode -q` 命令的输出格式.
 * 输出是有层次的
 * 
 * @param {*} cmd 
 * @param {RegExp} start_line_re 
 * @param {RegExp} skip_line_re 
 * @param {*} indent 缩进
 * @param {RegExp} split_re 
 * @returns {object} {}
 */
function execute_cmd_to_map2(cmd, start_line_re, skip_line_re, indent, split_re) {

    var info = {
    };

    let started = start_line_re === null,
        line,
        node,
        key,
        value
    ;
    let pipe = std.popen(cmd, "r");
    while ((line = pipe.getline()) != null) {
        // console.log('line', line);
        if(line.length==0){
            continue;
        }

        if(!started && start_line_re.test(line)){
            started = true;
            continue;
        }

        if(skip_line_re && skip_line_re.test(line)){
            continue;
        }

        let level2=0;
        while(line.indexOf(indent)==0){
            line = line.substr(indent.length);
            level2++;
        }

        if(level2==0){
            node = {};
            if(info[line]){
                if(info[line] instanceof Array){
                    info[line].push(node);
                }else{
                    info[line]=[info[line], node];
                }
            }else{
                info[line]=node;
            }
        }else if(level2==1){
            let part = line.split(split_re);
            key = part[0];
            value = part[1] === '' ? []: part[1];
            node[key] = value;
        }else if(level2==2){
            // console.log('value', value);
            if(typeof value !== 'array'){
                value = [];
                node[key] = value;
            }
            value.push(line);
        }else{
            console.log('unknown format:', line);
        }
    }
    pipe.close();

    return info;

}

function execute_cmd_to_array(cmd) {

    var info = [];

    let line
    ;
    let pipe = std.popen(cmd, "r");
    while ((line = pipe.getline()) != null) {
        info.push(line);
    }
    pipe.close();

    return info;

}
/**
 * 
 * @param {*} cmd 
 * @param {*} item_spliter_re 
 * @param {*} key_value_spliter_re 
 * @returns [{partN:valueN},...,{}]
 */
function execute_cmd_to_array_map(cmd, item_spliter_re, key_value_spliter_re) {

    var info = [];

    let line,
        item
    ;
    let pipe = std.popen(cmd, "r");
    while ((line = pipe.getline()) != null) {
        // console.log('line', line);
        if(item_spliter_re.test(line)){
            item={};
            info.push(item);
        }else{
            if (!item) {
                item={};
                info.push(item);
            }
            let part = line.split(key_value_spliter_re);
            item[part[0]] = part[1];
        }
    }
    pipe.close();

    return info;

}

function execute_cmd_to_table(cmd, re, with_header) {

    var info = {
        head: null,
        body: []
    };

    let line
    ;
    let pipe = std.popen(cmd, "r");
    while ((line = pipe.getline()) != null) {
        //console.log('line', line);
        if(line.length==0){
            continue;
        }
        let part = line.split(re);
        //console.log('part', part);
        if(with_header && info.head === null){
            info.head = part;
        }else{
            info.body.push(part);
        }
    }
    pipe.close();

    return info;

}

export default { 
    execute_cmd_to_map, 
    execute_cmd_to_map_array,
    execute_cmd_to_map2, 
    execute_cmd_to_array,
    execute_cmd_to_array_map,
    execute_cmd_to_table 
}