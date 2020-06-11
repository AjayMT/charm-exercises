
#include "key_value.decl.h"

#include <string>
#include <cstdlib>
#include <utility>

class Main : public CBase_Main
{
public:
  Main(CkArgMsg *m);
  void done(long sum) { ckout << sum << endl; CkExit(); }
};

Main::Main(CkArgMsg *msg)
{
  if (msg->argc < 4) {
    ckout << "3 arguments required" << endl;
    CkExit();
  }

  int m = std::atoi(msg->argv[1]);
  int n = std::atoi(msg->argv[2]);
  int k = std::atoi(msg->argv[3]);

  CProxy_A as = CProxy_A::ckNew(m, n, k, thisProxy, n);
}

class A : public CBase_A
{
  A_SDAG_CODE

  int m, n, k;
  long start_idx = 0;
  long size = 0;
  std::pair<long, long> *table = nullptr;
  std::pair<long, long> *requests = nullptr;
  int iter = 0;

  CProxy_Main main;

public:
  A(int sm, int sn, int sk, CProxy_Main m);
  ~A() { delete[] table; delete[] requests; }
  void request(int chare_idx, int table_idx, long key);
};

A::A(int sm, int sn, int sk, CProxy_Main s_main) : n(sn), m(sm), k(sk), main(s_main)
{
  size = m / n;
  start_idx = size * thisIndex;
  table = new std::pair<long, long>[size];
  for (int i = 0; i < size; ++i)
    table[i] = std::make_pair(start_idx + i, std::rand());

  requests = new std::pair<long, long>[k];
  for (int i = 0; i < k; ++i)
    requests[i] = std::make_pair(rand() % m, 0);

  thisProxy[thisIndex].run();
}

void A::request(int chare_idx, int table_idx, long key)
{
  thisProxy[chare_idx].response(table_idx, table[key - start_idx].second);
}

#include "key_value.def.h"
