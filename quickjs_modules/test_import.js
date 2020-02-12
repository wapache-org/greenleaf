// 在js里import模块的路径问题
// import a from './abc.js' # .开头 表示 相对于当前文件(模块)的路径
// import c from '/abc.js'  # /开头 表示 绝对路径
// import b from 'abc.js'   # 不是.和/开头 表示 相对于当前工作目录的路径
import {test_export} from "./test_export.js"

console.log("test_import invoke test_export: "+test_export());

