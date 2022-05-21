(lang dune 2.8)
(version 0.7.3)
(name magic)
(source (github savonet/ocaml-magic))
(license LGPL-2.1)
(authors "The Savonet Team <savonet-users@lists.sourceforge.net>")
(maintainers "Romain Beauxis <toots@rastageeks.org>")

(generate_opam_files true)
(use_standard_c_and_cxx_flags false)

(package
 (name magic)
 (synopsis "OCaml interface to libmagic.")
 (depends
  conf-magic
  conf-pkg-config
  dune
  dune-configurator)
)
