
#include <cmath>
#include <utility>
#include <cstdlib>
#include <cstdint>
#include "Primes.decl.h"

class Main : public CBase_Main
{
private:
  std::pair<uint64_t, bool> *primes;
  int count;
  int completed = 0;
  double start_time, end_time;
public:
  Main(CkArgMsg *m);
  void result(int index, int m, bool *arePrime);
};

Main::Main(CkArgMsg *msg)
{
  if (msg->argc < 3) {
    ckout << "2 arguments required" << endl;
    CkExit();
  }

  count = std::atoi(msg->argv[1]);
  int m = std::atoi(msg->argv[2]);
  primes = new std::pair<uint64_t, bool>[count];
  start_time = CkTimer();

  for (int i = 0; i < count; i += m) {
    uint64_t *nums = new uint64_t[m];
    int j = 0;
    for (; j < m && j + i < count; ++j) {
      uint64_t n = ((uint64_t)rand() << 32) | rand();
      primes[i + j] = std::make_pair(n, false);
      nums[j] = n;
    }
    CProxy_PrimeChecker::ckNew(i, j, nums, thisProxy);
    delete[] nums;
  }
}

void Main::result(int index, int m, bool *results)
{
  completed += m;
  for (int i = 0; i < m; ++i)
    primes[index + i].second = results[i];

  if (completed == count) {
    end_time = CkTimer();
    ckout << (end_time - start_time) << endl;
    delete[] primes;
    CkExit();
  }
}

class PrimeChecker : public CBase_PrimeChecker
{
private:
  int isPrime(const uint64_t number);
public:
  PrimeChecker(int index, int m, uint64_t *nums, CProxy_Main main);
};

int PrimeChecker::isPrime(const uint64_t number)
{
  if (number <= 1) return 0;
  if (number == 2) return 1;
  if (number % 2 == 0) return 0;

  for (int i = 3; i * i <= number; i += 2) {
    if (0 == number % i)
      return 0;
  }

  return 1;
}

PrimeChecker::PrimeChecker(int index, int m, uint64_t *nums, CProxy_Main main)
{
  bool *results = new bool[m];
  for (int i = 0; i < m; ++i)
    results[i] = isPrime(nums[i]);
  main.result(index, m, results);
  delete[] results;
}

#include "Primes.def.h"
