/**
 * usage:
 * 
 * var result = template('{{ d.name }}').render({
 *  name: 'abc'
 * });
 * 
 * template('{{ d.name }}').render({
 *  name: 'abc'
 * }, function(result){
 *   // ... ...
 * });
 * 
 */
var engine = (function(){
  var config = {
    open: '{{',
    close: '}}'
  };

  var tool = {
    exp: function(str){
      return new RegExp(str, 'g');
    },
    //匹配满足规则内容
    query: function(type, _, __){
      var types = [
        '#([\\s\\S])+?',   //js语句
        '([^{#}])*?' //普通字段
      ][type || 0];
      return exp((_||'') + config.open + types + config.close + (__||''));
    },   
    escape: function(html){
      return String(html||'').replace(/&(?!#?[a-zA-Z0-9]+;)/g, '&amp;')
      .replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/'/g, '&#39;').replace(/"/g, '&quot;');
    },
    error: function(e, tplog){
      var error = 'Template Error：';
      typeof console === 'object' && console.error(error + e + '\n'+ (tplog || ''));
      return error + e;
    }
  };

  var exp = tool.exp, Tpl = function(tpl){
    this.tpl = tpl;
  };

  Tpl.pt = Tpl.prototype;

  // window.errors = 0;

  //编译模版
  Tpl.pt.parse = function(tpl, data){
    var that = this, tplog = tpl;
    var jss = exp('^'+config.open+'#', ''), jsse = exp(config.close+'$', '');
    
    tpl = tpl
    .replace(/\n/g, '__~~n~~_')
    .replace(/\t/g, '__~~t~~_')
    .replace(/\s+|\r/g, ' ') // 换行制表符替换成空格  
    .replace(exp(config.open+'#'), config.open+'# ')
    .replace(exp(config.close+'}'), '} '+config.close)
    .replace(/\\/g, '\\\\')
    
    //不匹配指定区域的内容
    .replace(exp(config.open + '!(.+?)!' + config.close), function(str){
      str = str.replace(exp('^'+ config.open + '!'), '')
      .replace(exp('!'+ config.close), '')
      .replace(exp(config.open + '|' + config.close), function(tag){
        return tag.replace(/(.)/g, '\\$1')
      });
      return str
    })
    
    //匹配JS规则内容
    .replace(/(?="|')/g, '\\').replace(tool.query(), function(str){
      str = str.replace(jss, '').replace(jsse, '');
      return '";' + str.replace(/\\/g, '') + ';view+="';
    })
    
    //匹配普通字段
    .replace(tool.query(1), function(str){
      var start = '"+(';
      if(str.replace(/\s/g, '') === config.open+config.close){
        return '';
      }
      str = str.replace(exp(config.open+'|'+config.close), '');
      if(/^=/.test(str)){
        str = str.replace(/^=/, '');
        start = '"+_escape_(';
      }
      return start + str.replace(/\\/g, '') + ')+"';
    })
    .replace(/__~~n~~_/g, '\\n')
    .replace(/__~~t~~_/g, '\\t')
    ;
    
    tpl = '"use strict";var view = "' + tpl + '";return view;';
    //console.log(tpl);

    try{
      that.cache = tpl = new Function('d, _escape_', tpl);
      return tpl(data, tool.escape);
    } catch(e){
      delete that.cache;
      return tool.error(e, tplog);
    }
  };

  Tpl.pt.render = function(data, callback){
    var that = this, tpl;
    if(!data) return tool.error('no data');
    tpl = that.cache ? that.cache(data, tool.escape) : that.parse(that.tpl, data);
    if(!callback) return tpl;
    callback(tpl);
  };

  var eg = function(tpl){
    if(typeof tpl !== 'string') return tool.error('Template not found');
    return new Tpl(tpl);
  };

  eg.config = function(options){
    options = options || {};
    for(var i in options){
      config[i] = options[i];
    }
  };

  return eg;
})();

function get_template(tplstr) {
  return engine(tplstr);
}

var render = function (tplstr, data) {
  //if(console) console.log(data);
  return engine(tplstr).render(data);
}

// i18n translation
function _(str){
  return str;
}
