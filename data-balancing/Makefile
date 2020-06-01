
data-balancing: data_balancing.o
	charmc -o data-balancing data_balancing.o

data_balancing.o: data_balancing.cpp data_balancing.decl.h data_balancing.def.h
	charmc -O3 -c data_balancing.cpp

data_balancing.decl.h data_balancing.def.h: data_balancing.ci
	charmc data_balancing.ci

.PHONY: clean
clean:
	rm -f *.o data-balancing *.decl.h *.def.h

