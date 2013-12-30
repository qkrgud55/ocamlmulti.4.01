/***********************************************************************/
/*                                                                     */
/*                                OCaml                                */
/*                                                                     */
/*            Xavier Leroy, projet Cristal, INRIA Rocquencourt         */
/*                                                                     */
/*  Copyright 1996 Institut National de Recherche en Informatique et   */
/*  en Automatique.  All rights reserved.  This file is distributed    */
/*  under the terms of the GNU Library General Public License, with    */
/*  the special exception on linking described in file ../LICENSE.     */
/*                                                                     */
/***********************************************************************/

/* Raising exceptions from C. */

#include <signal.h>
#include "alloc.h"
#include "fail.h"
#include "io.h"
#include "gc.h"
#include "memory.h"
#include "mlvalues.h"
#include "printexc.h"
#include "signals.h"
#include "stack.h"
#include "roots.h"

/* The globals holding predefined exceptions */

typedef value caml_generated_constant[1];

extern caml_generated_constant
  caml_exn_Out_of_memory,
  caml_exn_Sys_error,
  caml_exn_Failure,
  caml_exn_Invalid_argument,
  caml_exn_End_of_file,
  caml_exn_Division_by_zero,
  caml_exn_Not_found,
  caml_exn_Match_failure,
  caml_exn_Sys_blocked_io,
  caml_exn_Stack_overflow,
  caml_exn_Assert_failure,
  caml_exn_Undefined_recursive_module;
extern caml_generated_constant
  caml_bucket_Out_of_memory,
  caml_bucket_Stack_overflow;

/* Exception raising */

extern void caml_raise_exception (pctx ctx, value bucket) Noreturn;

char * ctx->caml_exception_pointer = NULL;

void caml_raise(pctx ctx, value v)
{
  Unlock_exn();
  if (ctx->caml_exception_pointer == NULL) caml_fatal_uncaught_exception(ctx, v);

#ifndef Stack_grows_upwards
#define PUSHED_AFTER <
#else
#define PUSHED_AFTER >
#endif
  while (ctx->caml_local_roots != NULL &&
         (char *) ctx->caml_local_roots PUSHED_AFTER ctx->caml_exception_pointer) {
    ctx->caml_local_roots = ctx->caml_local_roots->next;
  }
#undef PUSHED_AFTER

  caml_raise_exception(ctx, v);
}

void caml_raise_constant(pctx ctx, value tag)
{
  CAMLparam1 (ctx, tag);
  CAMLlocal1 (ctx, bucket);

  bucket = caml_alloc_small (ctx, 1, 0);
  Field(bucket, 0) = tag;
  caml_raise(ctx, bucket);
  CAMLnoreturn;
}

void caml_raise_with_arg(pctx ctx, value tag, value arg)
{
  CAMLparam2 (ctx, tag, arg);
  CAMLlocal1 (ctx, bucket);

  bucket = caml_alloc_small (ctx, 2, 0);
  Field(bucket, 0) = tag;
  Field(bucket, 1) = arg;
  caml_raise(ctx, bucket);
  CAMLnoreturn;
}

void caml_raise_with_args(pctx ctx, value tag, int nargs, value args[])
{
  CAMLparam1 (ctx, tag);
  CAMLxparamN (ctx, args, nargs);
  value bucket;
  int i;

  Assert(1 + nargs <= Max_young_wosize);
  bucket = caml_alloc_small (ctx, 1 + nargs, 0);
  Field(bucket, 0) = tag;
  for (i = 0; i < nargs; i++) Field(bucket, 1 + i) = args[i];
  caml_raise(ctx, bucket);
  CAMLnoreturn;
}

void caml_raise_with_string(pctx ctx, value tag, char const *msg)
{
  caml_raise_with_arg(ctx, tag, caml_copy_string(ctx, msg));
}

void caml_failwith (pctx ctx, char const *msg)
{
  caml_raise_with_string(ctx, (value) caml_exn_Failure, msg);
}

void caml_invalid_argument (pctx ctx, char const *msg)
{
  caml_raise_with_string(ctx, (value) caml_exn_Invalid_argument, msg);
}

/* To raise [Out_of_memory], we can't use [caml_raise_constant],
   because it allocates and we're out of memory...
   We therefore use a statically-allocated bucket constructed
   by the ocamlopt linker.
   This works OK because the exception value for [Out_of_memory] is also
   statically allocated out of the heap.
   The same applies to Stack_overflow. */

void caml_raise_out_of_memory(pctx ctx)
{
  caml_raise(ctx, (value) &caml_bucket_Out_of_memory);
}

void caml_raise_stack_overflow(pctx ctx)
{
  caml_raise(ctx, (value) &caml_bucket_Stack_overflow);
}

void caml_raise_sys_error(pctx ctx, value msg)
{
  caml_raise_with_arg(ctx, (value) caml_exn_Sys_error, msg);
}

void caml_raise_end_of_file(pctx ctx)
{
  caml_raise_constant(ctx, (value) caml_exn_End_of_file);
}

void caml_raise_zero_divide(pctx ctx)
{
  caml_raise_constant(ctx, (value) caml_exn_Division_by_zero);
}

void caml_raise_not_found(pctx ctx)
{
  caml_raise_constant(ctx, (value) caml_exn_Not_found);
}

void caml_raise_sys_blocked_io(pctx ctx)
{
  caml_raise_constant(ctx, (value) caml_exn_Sys_blocked_io);
}

/* We allocate statically the bucket for the exception because we can't
   do a GC before the exception is raised (lack of stack descriptors
   for the ccall to [caml_array_bound_error].  */

#define BOUND_MSG "index out of bounds"
#define BOUND_MSG_LEN (sizeof(BOUND_MSG) - 1)

static struct {
  header_t hdr;
  value exn;
  value arg;
} array_bound_error_bucket;

static struct {
  header_t hdr;
  char data[BOUND_MSG_LEN + sizeof(value)];
} array_bound_error_msg = { 0, BOUND_MSG };

static int array_bound_error_bucket_inited = 0;

void caml_array_bound_error(pctx ctx)
{
  if (! ctx->array_bound_error_bucket_inited) {
    mlsize_t wosize = (BOUND_MSG_LEN + sizeof(value)) / sizeof(value);
    mlsize_t offset_index = Bsize_wsize(wosize) - 1;
    ctx->array_bound_error_msg.hdr = Make_header(wosize, String_tag, Caml_white);
    ctx->array_bound_error_msg.data[offset_index] = offset_index - BOUND_MSG_LEN;
    ctx->array_bound_error_bucket.hdr = Make_header(2, 0, Caml_white);
    ctx->array_bound_error_bucket.exn = (value) caml_exn_Invalid_argument;
    ctx->array_bound_error_bucket.arg = (value) ctx->array_bound_error_msg.data;
    ctx->array_bound_error_bucket_inited = 1;
    caml_page_table_add(In_static_data,
                        &ctx->array_bound_error_msg,
                        &ctx->array_bound_error_msg + 1);
    ctx->array_bound_error_bucket_inited = 1;
  }
  caml_raise(ctx, (value) &ctx->array_bound_error_bucket.exn);
}

int caml_is_special_exception(value exn) {
  return exn == (value) caml_exn_Match_failure
    || exn == (value) caml_exn_Assert_failure
    || exn == (value) caml_exn_Undefined_recursive_module;
}
