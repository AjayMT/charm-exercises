
#include "odd_even.decl.h"
#include <cstdlib>
#include <vector>


class Main : public CBase_Main
{
private:
  int n;
  CProxy_Elem elems;

public:
  Main(CkArgMsg *m);
  void done()
    { elems.verify(0, true); }
  void verified()
    { CkExit(); }
};

Main::Main(CkArgMsg *m)
{
  if (m->argc < 2) {
    ckout << "1 argument required" << endl;
    CkExit();
  }
  n = std::atoi(m->argv[1]);
  elems = CProxy_Elem::ckNew(thisProxy, n, n);
}


class Elem : public CBase_Elem
{
  Elem_SDAG_CODE;
private:
  CProxy_Main main;
  CProxyElement_Elem self;
  int n;
  int num;
  int stage = 0;

  // this chare's value should be greater than its partner's when:
  // - its index is even and it is on an odd stage
  //   (i.e its partner is the odd chare to its left)
  // - its index is odd and it is on an even stage
  //   (its partner is the even chare to its left)
  int should_be_greater()
    { return (stage + thisIndex) % 2 != 0; }

  int partner(int s)
    {
      int p = thisIndex + ((s + thisIndex) % 2 != 0 ? -1 : 1);
      if (p < 0) p = 0;
      if (p >= n) p = n - 1;
      return p;
    }

  void process_num(int d);

public:
  Elem(CProxy_Main m, int s);
  void verify(int d, bool is_main);
};

Elem::Elem(CProxy_Main m, int s) : main(m), n(s)
{
  self = thisProxy[thisIndex];
  num = rand();
  thisProxy[partner(stage)].recv_num(stage, num);
  self.do_sort();
}

void Elem::process_num(int d)
{
  if (stage >= n) {
    CkCallback cb(CkReductionTarget(Main, done), main);
    contribute(0, nullptr, CkReduction::nop, cb);
    return;
  }

  if ((should_be_greater() && num < d) || (!should_be_greater() && num > d))
    num = d;

  thisProxy[partner(stage + 1)].recv_num(stage + 1, num);
}

void Elem::verify(int d, bool is_main)
{
  if (is_main) {
    if (thisIndex < n - 1)
      thisProxy[thisIndex + 1].verify(num, false);

    if (thisIndex == 0) {
      CkCallback cb(CkReductionTarget(Main, verified), main);
      contribute(0, nullptr, CkReduction::nop, cb);
    }
    return;
  }

  if (d > num) {
    ckout << "did not sort correctly at " << thisIndex
          << " with values " << d << " " << num << endl;
    CkExit();
  }

  CkCallback cb(CkReductionTarget(Main, verified), main);
  contribute(0, nullptr, CkReduction::nop, cb);
}

#include "odd_even.def.h"
