
#include <cmath>
#include <utility>
#include <cstdlib>
#include "Primes.decl.h"

class Main : public CBase_Main
{
private:
  std::pair<long, bool> *primes;
  int count;
  int completed = 0;
public:
  Main(CkArgMsg *m);
  void result(int index, bool isPrime);
};

Main::Main(CkArgMsg *m)
{
  if (m->argc < 2) {
    ckout << "argument required" << endl;
    CkExit();
  }

  count = std::atoi(m->argv[1]);
  primes = new std::pair<long, bool>[count];
  for (int i = 0; i < count; ++i) {
    long n = rand();
    primes[i] = std::make_pair(n, false);
    CProxy_PrimeChecker::ckNew(i, n, thisProxy);
  }
}

void Main::result(int index, bool isPrime)
{
  ++completed;
  primes[index].second = isPrime;
  if (completed == count) {
    for (int i = 0; i < count; ++i)
      ckout
        << primes[i].first
        << (primes[i].second ? " is" : " is not")
        << " prime" << endl;
    delete[] primes;
    CkExit();
  }
}

class PrimeChecker : public CBase_PrimeChecker
{
private:
  int isPrime(const long number);
public:
  PrimeChecker(int index, long n, CProxy_Main main);
};

int PrimeChecker::isPrime(const long number)
{
  if (number <= 1) return 0;
  if (number == 2) return 1;

  for (int i = 3; i < number / 2; i += 2) {
    if (0 == number % i)
      return 0;
  }

  return 1;
}

PrimeChecker::PrimeChecker(int index, long n, CProxy_Main main)
{
  main.result(index, isPrime(n));
}

#include "Primes.def.h"
