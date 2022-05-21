(* File: magic.mli

   Copyright (C) 2005

     Christophe Troestler
     email: chris_77@users.sourceforge.net
     WWW: http://www.umh.ac.be/math/an/software/

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   version 2.1 as published by the Free Software Foundation, with the
   special exception on linking described in file LICENSE.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the file
   LICENSE for more details.
*)
(* 	$Id: magic.mli,v 1.7 2008/03/24 09:35:22 chris_77 Exp $	 *)

(** Try to identify the type of file using some "magic number" tests.

    @author Christophe Troestler <chris_77\@users.sourceforge.net>
    @version 0.7.3
*)

(** Magic "cookies". *)
type t

(** Raised by the functions of this library when they fail.  The
      string is an explanation of why the failure happened. *)
exception Failure of string

type flag =
  | Symlink  (** If the file queried is a symlink, follow it. *)
  | Compress
      (** If the file is compressed, unpack it and look at the
                    contents. *)
  | Devices
      (** If the file is a block or character special device,
                    then open the device and try to look in its
                    contents. *)
  | Mime  (** Return a mime string, instead of a textual description. *)
  | Continue  (** Return all matches, not just the first. *)
  | Check
      (** Check the magic database for consistency and print
		    warnings to stderr. *)
  | Preserve_atime
      (** On systems that support utime(2) or utimes(2),
		       attempt to preserve the access time of files
		       analyzed. *)
  | Raw
      (** Don't translate unprintable characters to a \ooo octal
		    representation. *)

(** [make ?flags filenames] creates a magic cookie, loading the
      databases in [filenames].  If [filenames = []], the default
      database is used.

      @param flags specifies how the other magic functions should
      behave (default: [[]]).
      @raise Magic.Failure if there was an error allocating the magic cookie.
      @raise Sys_error if there was an operating system error. *)
val make : ?flags:flag list -> string list -> t

(** @deprecated Synonym for {!Magic.make}. *)
val create : ?flags:flag list -> string list -> t

(** [close cookie] frees the resources associated with the cookie [c].
      You need not to close cookies as the resources will be freed by
      the garbage collector anyway. *)
val close : t -> unit

(** [file cookie filename] returns a textual description of the
      contents of the filename argument.

      @raise Magic.Failure if an error occurred.
      @raise Invalid_argument if the cookie is closed.
      @raise Sys_error if there was an operating system error. *)
val file : t -> string -> string

(** [file cookie ?len buf] returns a textual description of the
      contents of the filename argument.

      @raise Magic.Failure if an error occurred.
      @raise Invalid_argument if [cookie] is closed or if one does not
      have [0 <= len <= String.length buf].  *)
val buffer : t -> ?len:int -> string -> string

(** [setflags cookie flags] specifies how the other magic functions
      should behave.

      @raise Magic.Failure if the {!Magic.flag} [Preserve_atime] is
      not supported by the operating system.
      @raise Invalid_argument if the cookie has been closed. *)
val setflags : t -> flag list -> unit

(** [load cookie filenames] loads the the list of database files
      [filenames], or [] for the default database file.

      @raise Invalid_argument if the cookie has been closed.
      @raise Sys_error if there was an operating system error.  *)
val load : t -> string list -> unit

(** [compile cookie filenames] compiles the the list of database
      files [filenames], or [] for the default database.  The compiled
      files created are named from the basename(1) of each file
      argument with ".mgc" appended to it.

      @raise Invalid_argument if the cookie has been closed.
      @raise Sys_error if there was an operating system error. *)
val compile : t -> string list -> unit

(** [check cookie filenames] checks the validity of entries in the
      database files [filenames], or [] for the default database.

      @raise Invalid_argument if the cookie has been closed. *)
val check : t -> string list -> bool
