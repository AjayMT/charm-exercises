
#include <vector>
#include <cstdlib>
#include "data_balancing.decl.h"

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

  CProxy_DataContainer dcs = CProxy_DataContainer::ckNew(thisProxy, v, v);

  for (int i = 0; i < v; ++i) {
    int s = rand() % max_size;
    n += s;
    dcs[i].initialize_data(s);
  }
}

void Main::complete(int max)
{
  ckout << "max=" << max << " V=" << v << " N=" << n << endl;
  CkExit();
}


class DataContainer : public CBase_DataContainer
{
private:
  CProxy_Main main;
  int num_containers;
  int max_stage;
  bool parity = 0;
  std::vector<int> data;

  int stage = 0;
  std::vector<int> count_buffer;
  std::vector<int> flag_buffer;

  void next_stage();
  int partner();

public:
  DataContainer(CProxy_Main m, int n)
    : main(m), num_containers(n), max_stage((int)log2(n))
    {
      count_buffer.assign(max_stage, 0);
      flag_buffer.assign(max_stage, 0);
      for (int i = thisIndex; i >= 1; i -= 1 << (int)(log2(i)))
        parity = !parity;
    }
  void initialize_data(int s);
  void recv_count(int count, int from_stage);
  void recv_data(std::vector<int> data, int from_stage);
};

void DataContainer::initialize_data(int s)
{
  data.assign(s, thisIndex);
  stage = 1;
  next_stage();
}

void DataContainer::next_stage()
{
  if (stage > max_stage) {
    CkCallback cb(CkReductionTarget(Main, complete), main);
    int val = data.size();
    contribute(sizeof(int), &val, CkReduction::max_int, cb);
  }

  int p = partner();
  thisProxy[p].recv_count(data.size(), stage);

  if (flag_buffer[stage] == 1) { // count has been received from partner
    int mean = (data.size() + count_buffer[stage]) / 2;
    if (data.size() + count_buffer[stage] != mean * 2 && parity)
      ++mean;

    if (data.size() == mean) {
      ++stage;
      next_stage();
      return;
    } else if (count_buffer[stage] > data.size()) return;

    int diff = data.size() - mean;
    std::vector<int> temp(data.begin(), data.begin() + diff);
    data.erase(data.begin(), data.begin() + diff);
    thisProxy[p].recv_data(temp, stage);
    ++stage;
    next_stage();
  }
}

void DataContainer::recv_data(std::vector<int> d, int from_stage)
{
  data.insert(data.end(), d.begin(), d.end());
  ++stage;
  next_stage();
}

void DataContainer::recv_count(int count, int from_stage)
{
  if (from_stage != stage) {
    count_buffer[from_stage] = count;
    flag_buffer[from_stage] = 1;
    return;
  }

  int mean = (data.size() + count) / 2;
  if (data.size() + count != mean * 2 && parity)
    ++mean;

  if (data.size() == mean) {
    ++stage;
    next_stage();
    return;
  } else if (count > data.size()) return;

  int diff = data.size() - mean;
  std::vector<int> temp(data.begin(), data.begin() + diff);
  data.erase(data.begin(), data.begin() + diff);
  thisProxy[partner()].recv_data(temp, stage);
  ++stage;
  next_stage();
}

int DataContainer::partner()
{
  int bit = 1 << (stage - 1);
  if (thisIndex & bit) return thisIndex & (~bit);
  else return thisIndex | bit;
}

#include "data_balancing.def.h"
