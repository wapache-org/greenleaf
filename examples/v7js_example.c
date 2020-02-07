#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "v7.h"


static int v7_example(char* path);
static void render(struct v7 *v7, char* template, v7_val_t data);
static void call_engine_then_render(struct v7 *v7, char* template, v7_val_t data);

int
main(int argc, char* argv[])
{
    char* path = argc>1 ? argv[1] : "template_engine.js";
    return v7_example(path);
}

static const char* cs_engine_var_name = "engine";
static const char* cs_get_template_fn_name = "get_template";
static const char* cs_render_fn_name = "render";

static void call_engine_then_render(struct v7 *v7, char* template, v7_val_t data) {
  v7_val_t engine, tpl, func, result, args;

  engine = v7_get(v7, v7_get_global(v7), cs_engine_var_name, strlen(cs_engine_var_name));

  args = v7_mk_array(v7);
  v7_array_push(v7, args, v7_mk_string(v7, template, 0, 0));

  if (v7_apply(v7, func, V7_UNDEFINED, args, &tpl) == V7_OK) {
    func = v7_get(v7, tpl, cs_render_fn_name, strlen(cs_render_fn_name));
    args = v7_mk_array(v7);
    v7_array_push(v7, args, data);
    if (v7_apply(v7, func, tpl, args, &result) == V7_OK) {
      size_t len;
      const char * str = v7_get_string(v7, &result, &len);
      printf("Result: %d , %g\n", len, str);
    }
  } else {
    v7_print_error(stderr, v7, "Error while calling sum", result);
  }
}

static void render(struct v7 *v7, char* template, v7_val_t data) {
  v7_val_t func, result, args;

  func = v7_get(v7, v7_get_global(v7), cs_render_fn_name, strlen(cs_render_fn_name));

  args = v7_mk_array(v7);
  v7_array_push(v7, args, v7_mk_string(v7, template, strlen(template), 0));
  v7_array_push(v7, args, data);

  if (v7_apply(v7, func, V7_UNDEFINED, args, &result) == V7_OK) {
      size_t len;
      const char * str = v7_get_string(v7, &result, &len);
    printf("Result: %d , %s\n", len, str);
  } else {
    v7_print_error(stderr, v7, "Error while calling sum", result);
  }
}

static int v7_example(char* path) {
  enum v7_err rcode = V7_OK;
  v7_val_t result;
  struct v7 *v7 = v7_create();
  rcode = v7_exec_file(v7,path,&result);
  //rcode = v7_exec(v7, "var sum = function(a, b) { return a + b; };", &result);
  if (rcode != V7_OK) {
    v7_print_error(stderr, v7, "Evaluation error", result);
    goto clean;
  }

  {
    char* tpl = "the name is {{ d.name }}.";
    v7_val_t data = v7_mk_object(v7);
    v7_set(v7, data, "name", ~0, v7_mk_string(v7, "wapa", ~0, 0));

    render(v7, tpl, data);
  }

  {
    char* tpl = "<h2>{{= d.title }}</h2>";
    v7_val_t data = v7_mk_object(v7);
    v7_set(v7, data, "title", ~0, v7_mk_string(v7, "wapa</h2>", ~0, 0));

    render(v7, tpl, data);
  }

  {
    char* tpl = "{{# if(true){ }} start {{#  } else { }} ended {{#  } }} ";
    v7_val_t data = v7_mk_object(v7);
    v7_set(v7, data, "title", ~0, v7_mk_string(v7, "wapa", ~0, 0));

    render(v7, tpl, data);
  }

//   call_engine_then_render(v7, tpl, data);

clean:
  v7_destroy(v7);
  return (int) rcode;
}