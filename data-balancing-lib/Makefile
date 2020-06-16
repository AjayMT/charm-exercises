
CHARMC = $(CHARM_HOME)/bin/charmc

data-balancing: data_balancing.o
	$(CHARMC) -o data-balancing data_balancing.o

data_balancing.o: data_balancing.cpp data_balancing.decl.h data_balancing.def.h data_container.h data_container.decl.h data_container.def.h
	$(CHARMC) -O3 -c data_balancing.cpp

data_balancing.decl.h data_balancing.def.h data_container.decl.h data_container.def.h: data_balancing.ci data_container.ci
	$(CHARMC) data_container.ci
	$(CHARMC) data_balancing.ci

.PHONY: clean
clean:
	rm -f *.o data-balancing *.decl.h *.def.h

