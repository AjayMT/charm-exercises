
primes: Primes.o
	charmc -o primes Primes.o -tracemode projections

Primes.o: Primes.cpp Primes.decl.h Primes.def.h
	charmc -O3 -c Primes.cpp

Primes.decl.h Primes.def.h: primes.ci
	charmc primes.ci

.PHONY: clean
clean:
	rm -f *.o primes Primes.decl.h Primes.def.h
