/* File: magic.ml

   Copyright (C) 2005

     Christophe Troestler
     email: Christophe.Troestler@umh.ac.be
     WWW: http://www.umh.ac.be/math/an/software/

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   version 2.1 as published by the Free Software Foundation, with the
   special exception on linking described in file LICENSE.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the file
   LICENSE for more details.
*/
/* 	$Id: magic_stubs.c,v 1.2 2008/03/23 20:39:45 chris_77 Exp $	 */

#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>

#include <errno.h>
#include <magic.h>        /* man libmagic(3) */
#define _XOPEN_SOURCE 600 /* XSI-compliant strerror_r() */
#include <stdio.h>
#include <string.h>

#ifdef WANT_DEBUG
#define DEBUG(...)                                                             \
  fprintf(stderr, "DEBUG magic_stub: " __VA_ARGS__);                           \
  fprintf(stderr, "\n");                                                       \
  fflush(stderr)
#else
#define DEBUG(...)
#endif

#define CAML_MAGIC_VERSION "0.2"

/*
 * Failure
 */

/* Raise [Magic.Failure] with the message [msg].  */
static void raise_magic_failure(const char *msg) {
  static value *exn = NULL;
  if (exn == NULL)
    exn = (value *)caml_named_value("Magic.Failure");
  raise_with_string(*exn, (char *)msg);
}

/* [raise_on_error] raises an exception according to the error that
   happened at the last operation on [cookie].
   [fname] is the name of the calling function. */
static void raise_on_error(const char *fname, const magic_t cookie) {
  CAMLparam0();
  CAMLlocal1(verrmsg);
  static value *exn = NULL;
  const char *err_magic = magic_error(cookie);
  char *errmsg; /* For thread safety of error messages */
  const int flen = strlen(fname);

  DEBUG("errno=%i : %i : %s", errno, magic_errno(cookie), err_magic);

  if (err_magic != NULL) {
    /* libmagic error */
    if ((errmsg = malloc(flen + strlen(err_magic) + 1)) == NULL)
      raise_out_of_memory();
    if (exn == NULL)
      exn = (value *)caml_named_value("Magic.Failure");
    strcpy(errmsg, fname);
    strcpy(errmsg + flen, err_magic);
    verrmsg = copy_string(errmsg);
    free(errmsg); /* err_magic is freed by magic_close */
    raise_with_arg(*exn, verrmsg);
  } else {
    /* System error */
    const int err = magic_errno(cookie);
    int len = 80; /* buffer length */

    if ((errmsg = malloc(len)) == NULL)
      raise_out_of_memory();
    strcpy(errmsg, fname);
#ifdef HAVE_STRERROR_R
    /* Allocate buffer [errmsg] until there is enough space for the
     * error message. */
    while (strerror_r(err, errmsg + flen, len - flen) < 0) {
      /* Reallocate to a bigger size -- no need to keep the contents */
      len *= 2;
      free(errmsg);
      if ((errmsg = malloc(len)) == NULL)
        raise_out_of_memory();
      strcpy(errmsg, fname);
    }
#else
    strncat(errmsg, strerror(err), len - flen - 1);
#endif
    verrmsg = copy_string(errmsg);
    free(errmsg);
    raise_sys_error(verrmsg);
  }

  CAMLreturn0;
}

/*
 * magic_t
 */

/* magic_t is a pointer on 'struct magic_set' so one can set it to NULL */
#define COOKIE_VAL(v) (*((magic_t *)Data_custom_val(v)))

/* If the cookie has not been forcibly closed with [magic_close], free it. */
static void free_cookie(value c) {
  magic_t cookie = COOKIE_VAL(c);
  if (cookie != NULL) {
    magic_close(cookie);
    DEBUG("free_cookie: freeing cookie %p.", cookie);
    COOKIE_VAL(c) = NULL;
  }
}

/* compare magic_t pointers (=> total order) */
static int compare_cookie(value c1, value c2) {
  magic_t cookie1 = COOKIE_VAL(c1), cookie2 = COOKIE_VAL(c2);

  if (cookie1 == cookie2)
    return 0;
  else if (cookie1 < cookie2)
    return -1;
  else
    return 1;
}

static struct custom_operations cookie_ops = {
    /* identifier */ "be.ac.umh.math/magic.cookie." CAML_MAGIC_VERSION,
    /* finalize */ free_cookie,
    /* compare */ compare_cookie,
    /* hash */ custom_hash_default,
    /* serialize */ custom_serialize_default,
    /* deserialize */ custom_deserialize_default};

#define ALLOC_COOKIE                                                           \
  alloc_custom(&cookie_ops, sizeof(magic_t), sizeof(magic_t),                  \
               40 * sizeof(magic_t))

/*
 * Stubs
 */

CAMLprim value ocaml_magic_open(value flags) {
  CAMLparam1(flags);
  CAMLlocal2(c, verrmsg);
  char *errmsg;
  int len = 80;

  c = ALLOC_COOKIE;
  if ((COOKIE_VAL(c) = magic_open(Int_val(flags) | MAGIC_ERROR)) == NULL) {
    if (errno == EINVAL)
      /* An unsupported value for flags was given */
      raise_magic_failure("Magic.create: Preserve_atime not supported");
    else {
      const int err = errno; /* save it */

      if ((errmsg = malloc(len)) == NULL)
        raise_out_of_memory();
      strcpy(errmsg, "Magic.create: "); /* 14 chars */
#ifdef HAVE_STRERROR_R
      /* No cookie yet, so one cannot use the above generic err fun */
      while (strerror_r(err, errmsg + 14, len - 14) < 0) {
        len *= 2;
        free(errmsg);
        if ((errmsg = malloc(len)) == NULL)
          raise_out_of_memory();
        strcpy(errmsg, "Magic.create: ");
      }
#else
      strncat(errmsg, strerror(err), len - 15);
#endif
      verrmsg = copy_string(errmsg);
      free(errmsg);
      raise_sys_error(verrmsg);
    }
  }
  CAMLreturn(c);
}

CAMLprim value ocaml_magic_close(value c) {
  CAMLparam1(c);
  magic_t cookie = COOKIE_VAL(c);
  if (cookie != NULL) /* if first time it is called */
    magic_close(cookie);
  COOKIE_VAL(c) = NULL; /* For the finalization function & multiple calls */
  CAMLreturn(Val_unit);
}

CAMLprim value ocaml_magic_file(value c, value fname) {
  CAMLparam2(c, fname);
  const char *ans;
  const magic_t cookie = COOKIE_VAL(c);

  if (cookie == NULL)
    invalid_argument("Magic.file");
  if ((ans = magic_file(cookie, String_val(fname))) == NULL) {
    raise_on_error("Magic.file: ", cookie);
  }
  CAMLreturn(copy_string(ans));
}

CAMLprim value ocaml_magic_buffer(value c, value buf, value len) {
  CAMLparam3(c, buf, len);
  const char *ans;
  const magic_t cookie = COOKIE_VAL(c);

  if (cookie == NULL)
    caml_invalid_argument("Magic.buffer");
  if ((ans = magic_buffer(cookie, String_val(buf), Int_val(len))) == NULL)
    raise_on_error("Magic.buffer: ", cookie);
  CAMLreturn(copy_string(ans));
}

CAMLprim value ocaml_magic_setflags(value c, value flags) {
  CAMLparam2(c, flags);
  const magic_t cookie = COOKIE_VAL(c);

  if (cookie == NULL)
    caml_invalid_argument("Magic.setflags");
  if (magic_setflags(cookie, Int_val(flags)) < 0)
    raise_magic_failure("Magic.setflags: Preserve_atime not supported");
  CAMLreturn(Val_unit);
}

#define CHECK(fname)                                                           \
  const magic_t cookie = COOKIE_VAL(c);                                        \
                                                                               \
  if (cookie == NULL)                                                          \
    caml_invalid_argument("Magic.check");                                      \
  if (magic_check(cookie, fname) < 0)                                          \
    CAMLreturn(Val_false);                                                     \
  else                                                                         \
    CAMLreturn(Val_true)

CAMLprim value ocaml_magic_check_default(value c) {
  CAMLparam1(c);
  CHECK(NULL);
}
CAMLprim value ocaml_magic_check(value c, value filenames) {
  CAMLparam2(c, filenames);
  CHECK(String_val(filenames));
}

#define COMPILE(fname)                                                         \
  const magic_t cookie = COOKIE_VAL(c);                                        \
                                                                               \
  if (cookie == NULL)                                                          \
    caml_invalid_argument("Magic.compile");                                    \
  if (magic_compile(cookie, fname) < 0)                                        \
    raise_on_error("Magic.compile: ", cookie);                                 \
  CAMLreturn(Val_unit)

CAMLprim value ocaml_magic_compile_default(value c) {
  CAMLparam1(c);
  COMPILE(NULL);
}

CAMLprim value ocaml_magic_compile(value c, value filenames) {
  CAMLparam2(c, filenames);
  COMPILE(String_val(filenames));
}

#define LOAD(fname)                                                            \
  const magic_t cookie = COOKIE_VAL(c);                                        \
                                                                               \
  if (cookie == NULL)                                                          \
    caml_invalid_argument("Magic.load");                                       \
  if (magic_load(cookie, fname) < 0) {                                         \
    DEBUG("Magic.load: failed");                                               \
    /*    raise_on_error("Magic.load: ", cookie);*/                            \
    raise_magic_failure("Magic.load");                                         \
  }                                                                            \
  CAMLreturn(Val_unit)

CAMLprim value ocaml_magic_load_default(value c) {
  CAMLparam1(c);
  LOAD(NULL);
}

CAMLprim value ocaml_magic_load(value c, value filenames) {
  CAMLparam2(c, filenames);
  LOAD(String_val(filenames));
}
