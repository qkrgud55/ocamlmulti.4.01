#########################################################################
#                                                                       #
#                                 OCaml                                 #
#                                                                       #
#                 Xavier Clerc, SED, INRIA Rocquencourt                 #
#                                                                       #
#   Copyright 2010 Institut National de Recherche en Informatique et    #
#   en Automatique.  All rights reserved.  This file is distributed     #
#   under the terms of the Q Public License version 1.0.                #
#                                                                       #
#########################################################################

BASEDIR=../..

.PHONY: default
default: compile run

# phc : ocamlopt
.PHONY: compile
compile: caml
	@$(OCAMLOPT) -ccopt -I -ccopt . cmstub.c
	@$(OCAMLOPT) -ccopt -I -ccopt . cmmain.c
	@$(OCAMLOPT) -c cmcaml.ml
	@$(OCAMLOPT) -o program cmstub.$(O) cmcaml.cmx cmmain.$(O)

caml:
	@mkdir -p caml || :
	@cp -f $(TOPDIR)/byterun/*.h caml/

.PHONY: run
run:
	@printf " ... testing 'cmmain':"
	@./program >program.result
	@$(DIFF) program.reference program.result >/dev/null \
	&& echo " => passed" || echo " => failed"

.PHONY: promote
promote: defaultpromote

.PHONY: clean
clean: defaultclean
	@rm -f *.result program
	@rm -rf caml

include $(BASEDIR)/makefiles/Makefile.common
