
#include <cstdlib>
#include <vector>
#include <utility>
#include <cmath>
#include "k_means.decl.h"

struct Main : public CBase_Main
{
  int n, m, k;
  CProxy_Points points;
  double *centroid;
  int *counts = nullptr;
  double *coords = nullptr;

  Main(CkArgMsg *);
  void run();
  void update_counts(int *, int);
  void update_coords(double *, int);
  void update(int *, double *);
};

Main::Main(CkArgMsg *msg)
{
  if (msg->argc < 4) { CkPrintf("3 arguments required\n"); CkExit(); }

  n = std::atoi(msg->argv[1]);
  m = std::atoi(msg->argv[2]);
  k = std::atoi(msg->argv[3]);
  points = CProxy_Points::ckNew(n, m, thisProxy, n);
  centroid = new double[2 * k];
  for (int i = 0; i < k; ++i) {
    double x = static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
    double y = static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
    centroid[i << 1] = x;
    centroid[(i << 1) + 1] = y;
  }

  run();
}

void Main::run()
{
  counts = nullptr;
  coords = nullptr;
  points.assign(centroid, 2 * k);
}

void Main::update_counts(int *counts_, int n_counts)
{
  counts = counts_;
  if (coords) update(counts, coords);
}

void Main::update_coords(double *coords_, int n_coords)
{
  coords = coords_;
  if (counts) update(counts, coords);
}

void Main::update(int *counts, double *coords)
{
  double max_diff = -1;
  for (int i = 0; i < k; ++i) {
    if (counts[i] == 0) continue;
    double x_mean = coords[i << 1] / ((double)counts[i]);
    double y_mean = coords[(i << 1) + 1] / ((double)counts[i]);
    double x_diff = std::fabs(x_mean - centroid[i << 1]);
    double y_diff = std::fabs(y_mean - centroid[(i << 1) + 1]);
    double diff = std::fmax(x_diff, y_diff);
    if (max_diff == -1 || max_diff < diff) max_diff = diff;
    centroid[i << 1] = x_mean;
    centroid[(i << 1) + 1] = y_mean;
  }
  CkPrintf("%f\n", max_diff);
  if (max_diff <= 0.001) CkExit();
  run();
}

struct Points : CBase_Points
{
  int n_total_points;
  int n_points;
  int n_chares;
  std::vector<std::pair<double, double>> points;
  CProxy_Main main;

  Points(int n, int m, CProxy_Main);
  void assign(double *, int);
};

Points::Points(int n, int m, CProxy_Main main_)
  : n_total_points(m), n_points(m / n), n_chares(n), main(main_)
{
  points = std::vector<std::pair<double, double>>(n_points);
  for (int i = 0; i < n_points; ++i) {
    double x = static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
    double y = static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
    points[i] = std::make_pair(x, y);
  }
}

void Points::assign(double *centroid, int n_centroid_pairs)
{
  int n_centroid = n_centroid_pairs >> 1;
  int *assigned = new int[n_centroid];
  double *coords = new double[n_centroid_pairs];

  for (int i = 0; i < n_centroid; ++i) {
    assigned[i] = 0;
    coords[i << 1] = 0;
    coords[(i << 1) + 1] = 0;
  }

  for (int i = 0; i < n_points; ++i) {
    double min_dist = 0;
    int min_idx = -1;
    for (int j = 0; j < n_centroid; ++j) {
      double x = points[i].first - centroid[i << 1];
      double y = points[i].second - centroid[(i << 1) + 1];
      double dist = (x * x) + (y * y);
      if (min_idx == -1 || dist < min_dist) {
        min_dist = dist;
        min_idx = j;
      }
    }

    ++(assigned[min_idx]);
    coords[min_idx << 1] += points[i].first;
    coords[(min_idx << 1) + 1] += points[i].second;
  }

  CkCallback update_counts(CkReductionTarget(Main, update_counts), main);
  CkCallback update_coords(CkReductionTarget(Main, update_coords), main);
  contribute(n_centroid * sizeof(int), assigned, CkReduction::sum_int, update_counts);
  contribute(
    n_centroid_pairs * sizeof(double), coords, CkReduction::sum_double, update_coords
    );
}

#include "k_means.def.h"