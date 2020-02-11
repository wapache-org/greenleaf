import engine from 'template_engine.js'

var result = engine.template('{{ d.name }}').render({
 name: 'abc'
});
console.log('result1', result);

engine.template('{{ d.name }}').render({
 name: 'abc'
}, function(result){
    console.log('result2', result);
});

var options = {
  name: 'template1',
  open: '<%',
  close: '%>'
};
var template = engine.template('<% d.name %>', options);

result = template.render({
 name: 'abc'
});
console.log('result3', result);

result = engine.render('template1', {
 name: 'abc'
});
console.log('result4', result);