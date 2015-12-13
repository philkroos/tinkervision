Valgrinding tinkervision
========================

- The dlopen_vagrind program implements a minimal example showing that valgrind reports false negative for dlopen calls.

The other files in this directory are suppression files to be used when valgrinding tinkervision.  They have been created recursively with the command
`valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --gen-suppressions=all --suppressions=dlopen_any.supp --log-file=<file>.supp <program>`,
until no more false negatives were produced. The following valgrind reports are considered false negatives (google for e.g. valgrind dlopen):

- dlopen and related, which Valgrind interprets wrong
- libtbb (Intel Thread Building Blocks), which Valgrind interprets wrong
- libpython, which uses custom allocators

The produced suppression files were parsed with the awk script `parse_valgrind_suppressions.sh` (see first two links), then generalized with wildcards for each error type (the corresponding `*_any` files). All suppressions are collected in `tv_suppressions.supp`, so the preferred command to run valgrind (from any test) is

Reported `still reachable` memory is considered ok (see valgrind doc), so the complete preferred command to valgrind tinkervision is

`valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --show-reachable=no --suppressions=../valgrind_support/tv_suppressions.supp <program>`

# Possibly lost memory
This happens sometimes for threaded executions (e.g. in Dirwatch). Care has been taken that all threads are either detached (and *very* unlikely to not terminate eventually), or will be joined. Valgrind reports possibly lost memory anyways, but that's ok.

# Links
- https://wiki.wxwidgets.org/Valgrind_Suppression_File_Howto
- https://wiki.wxwidgets.org/Parse_valgrind_suppressions.sh
- http://svn.python.org/projects/python/trunk/Misc/valgrind-python.supp
- http://valgrind.org/docs/manual/mc-manual.html
- google valgrind dlopen
