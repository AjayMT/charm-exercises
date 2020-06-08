
particles: particles.o
	charmc -o particles particles.o -module liveViz -tracemode projections

particles.o: particles.cpp particles.decl.h particles.def.h
	charmc -O3 -c particles.cpp

particles.decl.h particles.def.h: particles.ci particles.h
	charmc particles.ci

.PHONY: clean
clean:
	rm -f *.o particles *.decl.h *.def.h

