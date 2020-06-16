
#include <vector>
#include <cstdlib>
#include "data_balancing.decl.h"
#include "data_container.h"

class Main : public CBase_Main
{
private:
  int v = 0, n = 0;

public:
  Main(CkArgMsg *m);
  void complete(int max);
};

Main::Main(CkArgMsg *m)
{
  if (m->argc < 3) {
    ckout << "2 arguments required" << endl;
    CkExit();
  }

  v = std::atoi(m->argv[1]);
  v = 1 << v;
  int max_size = std::atoi(m->argv[2]);

  CProxy_DataContainer<int> dc = CProxy_DataContainer<int>::ckNew(
    CkCallback(CkReductionTarget(Main, complete), thisProxy), v, v
    );
  int n = 0;
  for (int i = 0; i < v; ++i) {
    int size = rand() % max_size;
    n += size;
    std::vector<int> v; v.assign(size, i);
    dc[i].set_data(v);
  }
  ckout << "n = " << n << endl;

  dc.balance();
}

void Main::complete(int max)
{
  ckout << "max=" << max << " V=" << v << " N=" << n << endl;
  CkExit();
}

#include "data_balancing.def.h"
