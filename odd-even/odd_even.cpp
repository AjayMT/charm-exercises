
#include "odd_even.decl.h"
#include <cstdlib>
#include <vector>


class Main : public CBase_Main
{
private:
  int n;
  std::vector<int> sorted;

public:
  Main(CkArgMsg *m);
  void done(int idx, int num);
};

Main::Main(CkArgMsg *m)
{
  if (m->argc < 2) {
    ckout << "1 argument required" << endl;
    CkExit();
  }
  n = std::atoi(m->argv[1]);
  sorted.assign(n, 0);
  CProxy_Elem::ckNew(thisProxy, n, n);
}

void Main::done(int idx, int num)
{
  --n;
  sorted[idx] = num;
  if (n == 0) {
    for (int i : sorted) ckout << i << endl;
    CkExit();
  }
}

class Elem : public CBase_Elem
{
private:
  CProxy_Main main;
  int n;
  int num;
  int stage = 0;
  std::vector<int> data_buffer;
  std::vector<bool> flag_buffer;

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
};

Elem::Elem(CProxy_Main m, int s) : main(m), n(s)
{
  num = rand();
  data_buffer.assign(n, 0);
  flag_buffer.assign(n, false);
  next_stage();
}

void Elem::next_stage()
{
  if (stage >= n) {
    main.done(thisIndex, num);
    return;
  }

  thisProxy[partner(stage)].recv_num(num, stage);

  if (flag_buffer[stage]) {
    int d = data_buffer[stage];
    if ((should_be_greater() && num < d) || (!should_be_greater() && num > d))
      num = d;
    ++stage;
    next_stage();
  }
}

void Elem::recv_num(int d, int from)
{
  if (from != stage) {
    data_buffer[from] = d;
    flag_buffer[from] = true;
    return;
  }

  if ((should_be_greater() && num < d) || (!should_be_greater() && num > d))
    num = d;
  ++stage;
  next_stage();
}

#include "odd_even.def.h"
