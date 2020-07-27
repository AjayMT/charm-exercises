
k-means: k_means.o
	charmc -o k-means k_means.o

k_means.o: k_means.cpp k_means.decl.h k_means.def.h
	charmc -O3 -c k_means.cpp

k_means.decl.h k_means.def.h: k_means.ci
	charmc k_means.ci

.PHONY: clean
clean:
	rm -f *.o k-means *.decl.h *.def.h

