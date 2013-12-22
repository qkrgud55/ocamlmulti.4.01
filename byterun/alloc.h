/***********************************************************************/
/*                                                                     */
/*                                OCaml                                */
/*                                                                     */
/*         Xavier Leroy and Damien Doligez, INRIA Rocquencourt         */
/*                                                                     */
/*  Copyright 1996 Institut National de Recherche en Informatique et   */
/*  en Automatique.  All rights reserved.  This file is distributed    */
/*  under the terms of the GNU Library General Public License, with    */
/*  the special exception on linking described in file ../LICENSE.     */
/*                                                                     */
/***********************************************************************/

#ifndef CAML_ALLOC_H
#define CAML_ALLOC_H


#ifndef CAML_NAME_SPACE
#include "compatibility.h"
#endif
#include "misc.h"
#include "mlvalues.h"
#include "context.h"

#ifdef __cplusplus
extern "C" {
#endif

CAMLextern value caml_alloc (pctx ctx, mlsize_t, tag_t);
CAMLextern value caml_alloc_small (pctx ctx, mlsize_t, tag_t);
CAMLextern value caml_alloc_tuple (pctx ctx, mlsize_t);
CAMLextern value caml_alloc_string (pctx ctx, mlsize_t);  /* size in bytes */
CAMLextern value caml_copy_string (pctx ctx, char const *);
CAMLextern value caml_copy_string_array (pctx ctx, char const **);
CAMLextern value caml_copy_double (pctx ctx, double);
CAMLextern value caml_copy_int32 (pctx ctx, int32);       /* defined in [ints.c] */
CAMLextern value caml_copy_int64 (pctx ctx, int64);       /* defined in [ints.c] */
CAMLextern value caml_copy_nativeint (pctx ctx, intnat);  /* defined in [ints.c] */
CAMLextern value caml_alloc_array (pctx ctx, value (*funct) (char const *),
                                   char const ** array);

typedef void (*final_fun)(value);
CAMLextern value caml_alloc_final (pctx ctx, mlsize_t, /*size in words*/
                                   final_fun, /*finalization function*/
                                   mlsize_t, /*resources consumed*/
                                   mlsize_t  /*max resources*/);

CAMLextern int caml_convert_flag_list (value, int *);

#ifdef __cplusplus
}
#endif

#endif /* CAML_ALLOC_H */
