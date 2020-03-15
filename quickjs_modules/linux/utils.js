import * as std from 'std';

/**
 * 执行一条命令,并返回命令输出.
 * 
 * @param {string} cmd 命令
 * @returns {string} 命令输出
 */
function execute_cmd(cmd) {

    var info = [],
        line
    ;
    let pipe = std.popen(cmd, "r");
    while ((line = pipe.getline()) != null) {
        info.push(line);
    }
    pipe.close();

    return info.join('\n');
}

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
/**
 * 
 * @param {string} cmd 命令
 * @param {RegExp} re 列分割正则表达式
 * @param {boolean} with_header 命令的输出是否包含列名, 默认: false
 */
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

/**
 * 
 * 用于最后一列的内容可能包含分隔符的命令行输出.
 * 
 * @param {string} cmd 命令
 * @param {RegExp} split_re 列分割正则表达式
 * @param {string} split_char 最后一列内容合并填充字符
 * @param {boolean} with_header 输出内容是否包含表头
 * @param {number} column_count 列数
 */
function execute_cmd_to_table2(cmd, split_re, split_char, with_header, column_count) {

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
        let part = line.split(split_re);
        //console.log('part', part);
        if(with_header && info.head === null){
            info.head = part;
        }else{
            if(part.length > column_count){
                let arr = [];
                for (let i = 0; i < column_count-1; i++) {
                    arr[i] = part[i];
                    part[i] = "";
                }
                arr[column_count-1] = part.join(split_char).trim();

                info.body.push(arr);
            }else{
                info.body.push(part);
            }
        }
    }
    pipe.close();

    return info;

}


// 适用于对齐的输出, 譬如netstat -pnltu 
function lines_to_table(lines, split_char, column_count){

    let table = {
        head:[],
        body:[]
    };

    let max = lines
    .map(e=>e.length)
    .reduce((a,b)=>a>b?a:b, 0)
    ;

    let col_start=0, last_is_all_white_space=false;
    for (let i = 0; i < max; i++) {
        let all_white_space = lines
        .map(e=>e.length<i || e.charAt(i) === split_char ? true: false)
        .reduce((a,b)=>a && b, true)
        ;
        // console.log('i=', i, ', max=', max, ', all_white_space=', all_white_space, ', last_is_all_white_space=', last_is_all_white_space,', col_start=',col_start);
        if(all_white_space){

            // 列之间是多个空格, 忽略
            if(last_is_all_white_space){
                continue;
            }

            // 发现新组开始
            table.head.push(lines[0].substring(col_start, i).trim());
            for (let j = 1; j < lines.length; j++) {
                if(table.body.length<=j-1) {
                    table.body.push([]);
                }
                table.body[j-1].push(lines[j].substring(col_start, i).trim());
            }
            col_start = i;
        }
        last_is_all_white_space = all_white_space;

        if(column_count - table.head.length <= 1){

            table.head.push(lines[0].substring(col_start, lines[0].length).trim());
            for (let j = 1; j < lines.length; j++) {
                if(table.body.length<=j-1) {
                    table.body.push([]);
                }
                table.body[j-1].push(lines[j].substring(col_start, lines[j].length).trim());
            }

            break;
        }
    }
    return table;

}

export default { 
    execute_cmd,
    execute_cmd_to_map, 
    execute_cmd_to_map_array,
    execute_cmd_to_map2, 
    execute_cmd_to_array,
    execute_cmd_to_array_map,
    execute_cmd_to_table,
    execute_cmd_to_table2,
    lines_to_table
}