/***********************************************************************/
/*                                                                     */
/*                                OCaml                                */
/*                                                                     */
/*             Damien Doligez, projet Para, INRIA Rocquencourt         */
/*                                                                     */
/*  Copyright 1997 Institut National de Recherche en Informatique et   */
/*  en Automatique.  All rights reserved.  This file is distributed    */
/*  under the terms of the GNU Library General Public License, with    */
/*  the special exception on linking described in file ../LICENSE.     */
/*                                                                     */
/***********************************************************************/

/* Operations on weak arrays */

#include <string.h>

#include "alloc.h"
#include "fail.h"
#include "major_gc.h"
#include "memory.h"
#include "mlvalues.h"

value caml_weak_list_head = 0;

static value weak_dummy = 0;
value caml_weak_none = (value) &weak_dummy;

CAMLprim value caml_weak_create (pctx ctx, value len)
{
  mlsize_t size, i;
  value res;

  size = Long_val (len) + 1;
  if (size <= 0 || size > Max_wosize) caml_invalid_argument (ctx, "Weak.create");
  res = caml_alloc_shr (ctx, size, Abstract_tag);
  for (i = 1; i < size; i++) Field (res, i) = ctx->caml_weak_none;
  Field (res, 0) = ctx->caml_weak_list_head;
  ctx->caml_weak_list_head = res;
  return res;
}

#define None_val (Val_int(0))
#define Some_tag 0

static void do_set (pctx ctx, value ar, mlsize_t offset, value v)
{
  if (Is_block (v) && Is_young (v)){
    /* modified version of Modify */
    value old = Field (ar, offset);
    Field (ar, offset) = v;
    if (!(Is_block (old) && Is_young (old))){
      if (ctx->caml_weak_ref_table.ptr >= ctx->caml_weak_ref_table.limit){
        CAMLassert (ctx->caml_weak_ref_table.ptr == ctx->caml_weak_ref_table.limit);
        caml_realloc_ref_table (ctx, &ctx->caml_weak_ref_table);
      }
      *ctx->caml_weak_ref_table.ptr++ = &Field (ar, offset);
    }
  }else{
    Field (ar, offset) = v;
  }
}

CAMLprim value caml_weak_set (pctx ctx, value ar, value n, value el)
{
  mlsize_t offset = Long_val (n) + 1;
                                                   Assert (Is_in_heap (ar));
  if (offset < 1 || offset >= Wosize_val (ar)){
    caml_invalid_argument (ctx, "Weak.set");
  }
  if (el != None_val && Is_block (el)){
                                              Assert (Wosize_val (el) == 1);
    do_set (ctx, ar, offset, Field (el, 0));
  }else{
    Field (ar, offset) = ctx->caml_weak_none;
  }
  return Val_unit;
}

#define Setup_for_gc
#define Restore_after_gc

CAMLprim value caml_weak_get (pctx ctx, value ar, value n)
{
  CAMLparam2 (ctx, ar, n);
  mlsize_t offset = Long_val (n) + 1;
  CAMLlocal2 (ctx, res, elt);
                                                   Assert (Is_in_heap (ar));
  if (offset < 1 || offset >= Wosize_val (ar)){
    caml_invalid_argument (ctx, "Weak.get");
  }
  if (Field (ar, offset) == ctx->caml_weak_none){
    res = None_val;
  }else{
    elt = Field (ar, offset);
    if (ctx->caml_gc_phase == Phase_mark && Is_block (elt) && Is_in_heap (elt)){
      caml_darken (ctx, elt, NULL);
    }
    res = caml_alloc_small (ctx, 1, Some_tag);
    Field (res, 0) = elt;
  }
  CAMLreturn (ctx, res);
}

#undef Setup_for_gc
#undef Restore_after_gc

CAMLprim value caml_weak_get_copy (pctx ctx, value ar, value n)
{
  CAMLparam2 (ctx, ar, n);
  mlsize_t offset = Long_val (n) + 1;
  CAMLlocal2 (ctx, res, elt);
  value v;  /* Caution: this is NOT a local root. */
                                                   Assert (Is_in_heap (ar));
  if (offset < 1 || offset >= Wosize_val (ar)){
    caml_invalid_argument (ctx, "Weak.get");
  }

  v = Field (ar, offset);
  if (v == ctx->caml_weak_none) CAMLreturn (ctx, None_val);
  if (Is_block (v) && Is_in_heap_or_young(v)) {
    elt = caml_alloc (ctx, Wosize_val (v), Tag_val (v));
          /* The GC may erase or move v during this call to caml_alloc. */
    v = Field (ar, offset);
    if (v == ctx->caml_weak_none) CAMLreturn (ctx, None_val);
    if (Tag_val (v) < No_scan_tag){
      mlsize_t i;
      for (i = 0; i < Wosize_val (v); i++){
        value f = Field (v, i);
        if (ctx->caml_gc_phase == Phase_mark && Is_block (f) && Is_in_heap (f)){
          caml_darken (ctx, f, NULL);
        }
        Modify (&Field (elt, i), f);
      }
    }else{
      memmove (Bp_val (elt), Bp_val (v), Bosize_val (v));
    }
  }else{
    elt = v;
  }
  res = caml_alloc_small (ctx, 1, Some_tag);
  Field (res, 0) = elt;

  CAMLreturn (ctx, res);
}

CAMLprim value caml_weak_check (pctx ctx, value ar, value n)
{
  mlsize_t offset = Long_val (n) + 1;
                                                   Assert (Is_in_heap (ar));
  if (offset < 1 || offset >= Wosize_val (ar)){
    caml_invalid_argument (ctx, "Weak.get");
  }
  return Val_bool (Field (ar, offset) != ctx->caml_weak_none);
}

CAMLprim value caml_weak_blit (pctx ctx, value ars, value ofs,
                               value ard, value ofd, value len)
{
  mlsize_t offset_s = Long_val (ofs) + 1;
  mlsize_t offset_d = Long_val (ofd) + 1;
  mlsize_t length = Long_val (len);
  long i;
                                                   Assert (Is_in_heap (ars));
                                                   Assert (Is_in_heap (ard));
  if (offset_s < 1 || offset_s + length > Wosize_val (ars)){
    caml_invalid_argument (ctx, "Weak.blit");
  }
  if (offset_d < 1 || offset_d + length > Wosize_val (ard)){
    caml_invalid_argument (ctx, "Weak.blit");
  }
  if (ctx->caml_gc_phase == Phase_mark && ctx->caml_gc_subphase == Subphase_weak1){
    for (i = 0; i < length; i++){
      value v = Field (ars, offset_s + i);
      if (v != ctx->caml_weak_none && Is_block (v) && Is_in_heap (v)
          && Is_white_val (v)){
        Field (ars, offset_s + i) = ctx->caml_weak_none;
      }
    }
  }
  if (offset_d < offset_s){
    for (i = 0; i < length; i++){
      do_set (ctx, ard, offset_d + i, Field (ars, offset_s + i));
    }
  }else{
    for (i = length - 1; i >= 0; i--){
      do_set (ctx, ard, offset_d + i,  Field (ars, offset_s + i));
    }
  }
  return Val_unit;
}
