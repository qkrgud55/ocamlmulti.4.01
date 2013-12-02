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

.PHONY: compile
compile:
	@for f in *.ml; do \
	  F=`basename $$f .ml`; \
	  rm -f $$F.bytecode $$F.native $$F.native.exe; \
	  if $(NATIVE_ONLY); then : ; else \
	    $(OCAMLC) -w a -o $$F.bytecode $$f; \
	  fi; \
	  if $(BYTECODE_ONLY); then : ; else \
	    $(OCAMLOPT) -w a -o $$F.native$(EXE) $$f; \
	  fi; \
	done
	@grep -q HAS_STACK_OVERFLOW_DETECTION $(TOPDIR)/config/s.h \
	  || rm -f stackoverflow.native$(EXE)

.PHONY: run
run:
	@ulimit -s 1024; \
	 if $(NATIVE_ONLY); then : ; else \
	 for f in *.bytecode; do \
	   printf " ... testing '$$f':"; \
	   $(OCAMLRUN) ./$$f >$$f.result 2>&1 || true; \
	   DIFF="$(DIFF)" sh $$f.checker \
	   && echo " => passed" || echo " => failed"; \
	 done; \
	 fi; \
	 for fn in *.native; do \
	   if $(BYTECODE_ONLY) ; then : ; else \
	     printf " ... testing '$$fn':"; \
	     ./$${fn}$(EXE) >$$fn.result 2>&1 || true; \
	     DIFF="$(DIFF)" sh $$fn.checker \
	     && echo " => passed" || echo " => failed"; \
	   fi; \
	 done

.PHONY: promote
promote: defaultpromote

.PHONY: clean
clean: defaultclean
	@rm -f *.bytecode *.native *.native.exe *.result

include $(BASEDIR)/makefiles/Makefile.common
