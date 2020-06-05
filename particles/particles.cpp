
#include "particles.decl.h"
#include <cstdlib>
#include <vector>
#include <utility>
#include <cstring>
#include <cmath>
#include <random>
#include <algorithm>

#define MAX_ITER 100

class Main : public CBase_Main
{
private:
  CProxy_Box boxes;
  int k;

public:
  Main(CkArgMsg *m);
  void gather(CkReductionMsg *m);
  void done() { CkExit(); }
};

Main::Main(CkArgMsg *m)
{
  if (m->argc < 3) {
    ckout << "2 arguments required" << endl;
    CkExit();
  }

  int n = std::atoi(m->argv[1]);
  k = std::atoi(m->argv[2]);
  boxes = CProxy_Box::ckNew(n, k, thisProxy, k, k);
}

void Main::gather(CkReductionMsg *msg)
{
  CkReduction::tupleElement *results;
  int num_reductions;
  msg->toTuple(&results, &num_reductions);
  int max = *(int *)results[0].data;
  int sum = *(int *)results[1].data;
  ckout << "max: " << max
        << " average: " << ((double)sum / (double)(k * k)) << endl;
  delete[] results;
}

class Box : public CBase_Box
{
  Box_SDAG_CODE

  CProxy_Main main;

  int n, k;
  double x, y, size;
  int iter = 0;
  int sub_iter = 0;

  std::vector<std::pair<double, double>> particles;

  void update_particles();

public:
  Box(int sn, int sk, CProxy_Main m);
};

Box::Box(int sn, int sk, CProxy_Main m) : n(sn), k(sk), main(m)
{
  size = 100.0 / (double)k;
  x = thisIndex.x * size;
  y = thisIndex.y * size;

  std::random_device rd;
  std::mt19937_64 e2(rd());
  std::uniform_real_distribution<> dist(0, size);
  for (int i = 0; i < n; ++i) {
    std::pair<double, double> p = std::make_pair(x + dist(e2), y + dist(e2));
    particles.push_back(p);
  }

  thisProxy(thisIndex.x, thisIndex.y).run();
}

void Box::update_particles()
{
  std::random_device rd;
  std::mt19937_64 e2(rd());
  std::uniform_real_distribution<> dist(-size / 10.0, size / 10.0);

  std::vector<int> outside[3][3];
  std::vector<std::pair<double, double>> next_particles;

  for (int i = 0; i < particles.size(); ++i) {
    particles[i].first += dist(e2);
    particles[i].second += dist(e2);

    int px = (int)std::floor(particles[i].first / size);
    int py = (int)std::floor(particles[i].second / size);

    if (px == thisIndex.x && py == thisIndex.y) {
      next_particles.push_back(particles[i]);
      continue;
    }

    particles[i].first = fmod(particles[i].first, 100.0);
    particles[i].second = fmod(particles[i].second, 100.0);
    if (particles[i].first < 0) particles[i].first += 100.0;
    if (particles[i].second < 0) particles[i].second += 100.0;

    px -= thisIndex.x - 1; py -= thisIndex.y - 1;
    outside[px][py].push_back(i);
  }

  for (int i = 0; i < 9; ++i) {
    int tx = i % 3; int ty = i / 3;
    if (tx == 1 && ty == 1) continue; // ignore self

    std::vector<int> p_idxs = outside[tx][ty];
    std::vector<std::pair<double, double>> ps;
    for (int j : p_idxs) ps.push_back(particles[j]);

    tx = (tx + thisIndex.x - 1) % k;
    ty = (ty + thisIndex.y - 1) % k;
    if (tx < 0) tx += k;
    if (ty < 0) ty += k;

    thisProxy(tx, ty).recv_particle(iter, ps);
  }

  particles = next_particles;
}

#include "particles.def.h"
