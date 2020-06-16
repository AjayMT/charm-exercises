
#ifndef _DATA_CONTAINER_H_
#define _DATA_CONTAINER_H_

#include "data_container.decl.h"
#include <vector>

template <typename DType>
class DataContainer : public CBase_DataContainer<DType>
{
  DataContainer_SDAG_CODE

  CkCallback cb;
  int num_containers;
  std::vector<DType> data;
  std::vector<DType> balanced_data;
  int prefix_sum = 0;
  int count = 0;
  int stage = 1;
  int total = 0;
  int mean = 0;

  void balance_data();
  void check_balanced();
  int should_round_up(int idx)
    {
      return idx < total - (mean * num_containers) && mean * num_containers != total;
    }

public:
  DataContainer(CkCallback c, int n) : cb(c), num_containers(n) {}
  void set_data(std::vector<DType> d) { data = d; prefix_sum = count = data.size(); }
  void recv_data(std::vector<DType> d);
};

template <typename DType>
void DataContainer<DType>::recv_data(std::vector<DType> d)
{
  balanced_data.insert(balanced_data.end(), d.begin(), d.end());
  check_balanced();
}

template <typename DType>
void DataContainer<DType>::check_balanced()
{
  if (
    (balanced_data.size() == mean && !should_round_up(this->thisIndex))
    || (balanced_data.size() == mean + 1 && should_round_up(this->thisIndex))
    )
  {
    int val = balanced_data.size();
    this->contribute(sizeof(int), &val, CkReduction::max_int, cb);
  }
}

template <typename DType>
void DataContainer<DType>::balance_data()
{
  prefix_sum -= count;

  int target = prefix_sum / mean;
  int round_up_diff = total - (mean * num_containers);
  if (round_up_diff) {
    if (this->thisIndex < round_up_diff)
      target = prefix_sum / (mean + 1);
    else {
      int p = prefix_sum - ((mean + 1) * round_up_diff);
      target = round_up_diff + (p / mean);
    }
  }

  int step = 0;
  for (int i = 0; i < count && target < num_containers; i += step, ++target) {
    int rud = total - (mean * num_containers);
    if (rud) rud = rud < target + 1 ? rud : target + 1;

    step = (((target + 1) * mean) + rud) - (prefix_sum + i);
    if (step == 0) continue;

    int end = i + step > count ? count : i + step;
    if (end > count) end = count;

    std::vector<int> v(data.begin() + i, data.begin() + end);
    this->thisProxy[target].recv_data(v);
  }

  check_balanced();
}

#define CK_TEMPLATES_ONLY
#include "data_container.def.h"
#undef CK_TEMPLATES_ONLY

#endif /* _DATA_CONTAINER_H_ */
