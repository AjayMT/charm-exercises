
key-value: key_value.o
	charmc -o key-value key_value.o

key_value.o: key_value.cpp key_value.decl.h key_value.def.h
	charmc -O3 -c key_value.cpp

key_value.decl.h key_value.def.h: key_value.ci
	charmc key_value.ci

.PHONY: clean
clean:
	rm -f *.o key-value *.decl.h *.def.h

