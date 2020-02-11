/**
 * usage: see template_engine_test.js
 */
var 
  Template = function(options){
    this.options = options;
  }
;
Template.prototype = {
  options: null,
  renderFunc: null,
  // 编译模版
  compile: function(){

    var 
      code = (this.options.template || this.options.tpl || ''), 
      open = this.options.open, 
      close = this.options.close,
      keepFormat = this.options.keepFormat
    ;

    var 
      exp = function(str){
        return new RegExp(str, 'g');
      },
      //匹配满足规则内容
      query = function(type, _, __){
        var types = [
          '#([\\s\\S])+?',   //js语句
          '([^{#}])*?' //普通字段
        ][type || 0];
        return exp((_||'') + open + types + close + (__||''));
      }
    ;

    var
      jss  = exp('^'+open+'#', ''), 
      jsse = exp(close+'$', '')
    ;

    if(keepFormat){
      code = code
      .replace(/\n/g, '__~~n~~_')
      .replace(/\t/g, '__~~t~~_')
      ;
    }

    code = code
    .replace(/\s+|\r/g, ' ') // 换行制表符替换成空格  
    .replace(exp(open+'#'), open+'# ')
    .replace(exp(close+'}'), '} '+close)
    .replace(/\\/g, '\\\\')
    
    //不匹配指定区域的内容
    .replace(exp(open + '!(.+?)!' + close), function(str){
      str = str.replace(exp('^'+ open + '!'), '')
      .replace(exp('!'+ close), '')
      .replace(exp(open + '|' + close), function(tag){
        return tag.replace(/(.)/g, '\\$1')
      });
      return str;
    })
    
    //匹配JS规则内容
    .replace(/(?="|')/g, '\\')
    .replace(query(), function(str){
      str = str
      .replace(jss, '')
      .replace(jsse, '')
      .replace(/\\/g, '')
      ;
      return '";' + str+ ';view+="';
    })
    
    //匹配普通字段
    .replace(query(1), function(str){
      var start = '"+(';
      if(str.replace(/\s/g, '') === open+close){
        return '';
      }
      str = str.replace(exp(open+'|'+close), '');
      if(/^=/.test(str)){
        str = str.replace(/^=/, '');
        start = '"+_escape_(';
      }
      return start + str.replace(/\\/g, '') + ')+"';
    })
    ;

    if(keepFormat){
      code = code
      .replace(/__~~n~~_/g, '\\n')
      .replace(/__~~t~~_/g, '\\t')
      ;
    }
    
    code = '"use strict";var view = "' + code + '";return view;';
    //console.log(code);
  
    this.renderFunc = new Function('d, _escape_, _', code);

  },
  render : function(data, callback){
    if(!data) return this.options.error('no data');
    try{
      if(!this.renderFunc) this.compile();
      var result = this.renderFunc(data, this.options.escape, this.options.i18n);
      if(callback){
        callback(result);
      }else{
        return result;
      }
    } catch(e){
      return this.options.error(e, tplog);
    }
  }
};

var engine = {
  options: {
    open: '{{',
    close: '}}',
    debug: false,
    keepFormat: false,
    escape: function(html){
      return String(html||'')
      .replace(/&(?!#?[a-zA-Z0-9]+;)/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/'/g, '&#39;')
      .replace(/"/g, '&quot;')
      ;
    },
    // i18n translation
    i18n: function (str){
      return str;
    },
    error: function(e, tplog){
      var error = 'Template Error：';
      typeof console === 'object' && console.error && console.error(error + e + '\n'+ (tplog || ''));
      return error + e;
    }
  },
  templates: {},
  template: function(template, options) {
    var opt = {};

    if (typeof template === 'object') {
      for(var p in template) opt[p]=template[p];
    } else {
      for(var p in options) opt[p]=options[p];
      opt.template = template;
    }

    for(var p in engine.options)
      if(opt[p]==undefined) opt[p]=engine.options[p];

    if (opt.debug) typeof console === 'object' && console.log && console.log(JSON.stringify(opt));

    var t = new Template(opt);
    if(opt.name)
      this.templates[opt.name] = t;
    
    return t;
  },
  render: function(name, data, callback) {
    if( this.templates[name])
      return this.templates[name].render(data, callback);
    else {
      var result = this.options.error('Template not found', name);
      if(callback)
        callback(result);
      return result;
    }
  }
};

export default engine;
