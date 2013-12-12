
test: test_fun.o test.cmx
	./ocamlopt -o test test.cmx test_fun.o

test_fun.o: test_fun.c
	gcc -c test_fun.c

test.cmx: test.ml
	./ocamlopt -c test.ml

.PHONY: test test.cmx test_fun.o

clean:
	rm *.o *.cmx test

