
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
private:
  CProxy_Main main;
  int n;
  int num;
  int stage = 0;
  int next_data = 0;
  bool next_flag = false;

  void next_stage();

  // this chare's value should be greater than its partner's when:
  // - its index is even and it is on an odd stage
  //   (i.e its partner is the odd chare to its left)
  // - its index is odd and it is on an even stage
  //   (its partner is the even number to its left)
  int should_be_greater()
    { return (stage + thisIndex) % 2 != 0; }

  int partner(int s)
    {
      int p = thisIndex + (should_be_greater() ? -1 : 1);
      if (p < 0) p = 0;
      if (p >= n) p = n - 1;
      return p;
    }

public:
  Elem(CProxy_Main m, int s);
  void recv_num(int d, int from);
  void verify(int d, bool is_main);
};

Elem::Elem(CProxy_Main m, int s) : main(m), n(s)
{
  num = rand();
  next_stage();
}

void Elem::next_stage()
{
  if (stage >= n) {
    CkCallback cb(CkReductionTarget(Main, done), main);
    contribute(0, nullptr, CkReduction::nop, cb);
    return;
  }

  thisProxy[partner(stage)].recv_num(num, stage);

  if (next_flag) {
    next_flag = false;
    if (
      (should_be_greater() && num < next_data)
      || (!should_be_greater() && num > next_data)
      )
      num = next_data;
    ++stage;
    next_stage();
  }
}

void Elem::recv_num(int d, int from)
{
  if (from != stage) {
    next_data = d;
    next_flag = true;
    return;
  }

  if ((should_be_greater() && num < d) || (!should_be_greater() && num > d))
    num = d;
  ++stage;
  next_stage();
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
