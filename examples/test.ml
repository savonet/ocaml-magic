(* Magic script to execute in the toplevel *)
#directory "../src"

#load "magic.cma"

open Printf;;

printf "\n**********OCaml script starts*************************\n%!"

(* let c = Magic.make ["/usr/share/file/magic"];; *)
let c = Magic.make []
let m1 = Magic.file c "../src/dllmagic_stubs.so"
let m2 = Magic.file c "file.ml"
let m3 = Magic.file c "/dev/null"
let m4 = Magic.file c "xxx"
let m5 = Magic.file c "../src/magic.cmxa";;

Magic.load c ["/etc/magic"];;

(* usually empty *)
Magic.load c ["/usr/share/file/magic"];;
Magic.load c ["/usr/share/file/magic.mime"];;
Magic.close c
