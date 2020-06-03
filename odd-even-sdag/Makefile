
odd-even: odd_even.o
	charmc -o odd-even odd_even.o

odd_even.o: odd_even.cpp odd_even.decl.h odd_even.def.h
	charmc -O3 -c odd_even.cpp

odd_even.decl.h odd_even.def.h: odd_even.ci
	charmc odd_even.ci

.PHONY: clean
clean:
	rm -f *.o odd-even *.decl.h *.def.h

