/***********************************************************************/
/*                                                                     */
/*                                OCaml                                */
/*                                                                     */
/*            Alain Frisch, projet Gallium, INRIA Rocquencourt         */
/*                                                                     */
/*  Copyright 2007 Institut National de Recherche en Informatique et   */
/*  en Automatique.  All rights reserved.  This file is distributed    */
/*  under the terms of the GNU Library General Public License, with    */
/*  the special exception on linking described in file ../LICENSE.     */
/*                                                                     */
/***********************************************************************/

#include "misc.h"
#include "mlvalues.h"
#include "memory.h"
#include "stack.h"
#include "callback.h"
#include "alloc.h"
#include "intext.h"
#include "osdeps.h"
#include "fail.h"

#include <stdio.h>
#include <string.h>

static void *getsym(void *handle, char *module, char *name){
  char *fullname = malloc(strlen(module) + strlen(name) + 5);
  void *sym;
  sprintf(fullname, "caml%s%s", module, name);
  sym = caml_dlsym (handle, fullname);
  /*  printf("%s => %lx\n", fullname, (uintnat) sym); */
  free(fullname);
  return sym;
}

extern char caml_globals_map[];

CAMLprim value caml_natdynlink_getmap(value unit)
{
  return (value)caml_globals_map;
}

CAMLprim value caml_natdynlink_globals_inited(value unit)
{
  return Val_int(caml_globals_inited);
}

CAMLprim value caml_natdynlink_open(value filename, value global)
{
  CAMLparam1 (ctx, filename);
  CAMLlocal1 (ctx, res);
  void *sym;
  void *handle;

  /* TODO: dlclose in case of error... */

  handle = caml_dlopen(String_val(filename), 1, Int_val(global));

  if (NULL == handle)
    CAMLreturn(ctx, caml_copy_string(ctx, caml_dlerror()));

  sym = caml_dlsym(handle, "caml_plugin_header");
  if (NULL == sym)
    CAMLreturn(ctx, caml_copy_string(ctx, "not an OCaml plugin"));

  res = caml_alloc_tuple(ctx, 2);
  Field(res, 0) = (value) handle;
  Field(res, 1) = (value) (sym);
  CAMLreturn(ctx, res);
}

CAMLprim value caml_natdynlink_run(void *handle, value symbol) {
  CAMLparam1 (ctx, symbol);
  CAMLlocal1 (ctx, result);
  void *sym,*sym2;
  struct code_fragment * cf;

#define optsym(n) getsym(handle,unit,n)
  char *unit;
  void (*entrypoint)(void);

  unit = String_val(symbol);

  sym = optsym("__frametable");
  if (NULL != sym) caml_register_frametable(ctx, sym);

  sym = optsym("");
  if (NULL != sym) caml_register_dyn_global(ctx, sym);

  sym = optsym("__data_begin");
  sym2 = optsym("__data_end");
  if (NULL != sym && NULL != sym2)
    caml_page_table_add(In_static_data, sym, sym2);

  sym = optsym("__code_begin");
  sym2 = optsym("__code_end");
  if (NULL != sym && NULL != sym2) {
    caml_page_table_add(In_code_area, sym, sym2);
    cf = caml_stat_alloc(ctx, sizeof(struct code_fragment));
    cf->code_start = (char *) sym;
    cf->code_end = (char *) sym2;
    cf->digest_computed = 0;
    caml_ext_table_add(&caml_code_fragments_table, cf);
  }

  entrypoint = optsym("__entry");
  if (NULL != entrypoint) result = caml_callback(ctx, (value)(&entrypoint), 0);
  else result = Val_unit;

#undef optsym

  CAMLreturn (ctx, result);
}

CAMLprim value caml_natdynlink_run_toplevel(value filename, value symbol)
{
  CAMLparam2 (ctx, filename, symbol);
  CAMLlocal2 (ctx, res, v);
  void *handle;

  /* TODO: dlclose in case of error... */

  handle = caml_dlopen(String_val(filename), 1, 1);

  if (NULL == handle) {
    res = caml_alloc(ctx, 1,1);
    v = caml_copy_string(ctx, caml_dlerror());
    Store_field(ctx, res, 0, v);
  } else {
    res = caml_alloc(ctx, 1,0);
    v = caml_natdynlink_run(handle, symbol);
    Store_field(ctx, res, 0, v);
  }
  CAMLreturn(ctx, res);
}

CAMLprim value caml_natdynlink_loadsym(value symbol)
{
  CAMLparam1 (ctx, symbol);
  CAMLlocal1 (ctx, sym);

  sym = (value) caml_globalsym(String_val(symbol));
  if (!sym) caml_failwith(ctx, String_val(symbol));
  CAMLreturn(ctx, sym);
}
