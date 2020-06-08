
#include "liveViz.h"
#include "particles.decl.h"
#include <cstdlib>
#include <vector>
#include <utility>
#include <cstring>
#include <cmath>
#include <random>
#include <algorithm>

#define MAX_ITER 1000

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
  CkArrayOptions opts(k, k);
  boxes = CProxy_Box::ckNew(n, k, thisProxy, opts);

  CkCallback cb(CkIndex_Box::render(0), boxes);
  liveVizConfig cfg(liveVizConfig::pix_color, true);
  liveVizInit(cfg, boxes, cb, opts);

  boxes.run();
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

  std::vector<Particle> particles;

  void update_particles();
  void generate_particles(Particle::color_t c, int num);

public:
  Box(int sn, int sk, CProxy_Main m);
  void render(liveVizRequestMsg *m);
};

Box::Box(int sn, int sk, CProxy_Main m) : n(sn), k(sk), main(m)
{
  size = 100.0 / (double)k;
  x = thisIndex.x * size;
  y = thisIndex.y * size;

  std::random_device rd;
  std::mt19937_64 e2(rd());
  std::uniform_real_distribution<> dist(0, size);

  if (
    thisIndex.x * 4 >= k && thisIndex.x * 4 <= 3 * k
    && thisIndex.y * 4 >= k && thisIndex.y * 4 <= 3 * k
    )
    generate_particles(Particle::RED, n * 2);

  if (thisIndex.x + thisIndex.y <= k)
    generate_particles(Particle::GREEN, n);
  if (thisIndex.x + thisIndex.y >= k)
    generate_particles(Particle::BLUE, n);

  Particle::color_t c = Particle::RED;
  for (int i = 0; i < n; ++i) {
    Particle p(x + dist(e2), y + dist(e2), c);
    particles.push_back(p);
  }
}

void Box::generate_particles(Particle::color_t c, int num)
{
  std::random_device rd;
  std::mt19937_64 e2(rd());
  std::uniform_real_distribution<> dist(0, size);

  for (int i = 0; i < num; ++i) {
    Particle p(x + dist(e2), y + dist(e2), c);
    particles.push_back(p);
  }

}

void Box::update_particles()
{
  std::random_device rd;
  std::mt19937_64 e2(rd());
  std::uniform_real_distribution<> dist(-size / 10.0, size / 10.0);

  std::vector<int> outside[3][3];
  std::vector<Particle> next_particles;


  for (int i = 0; i < particles.size(); ++i) {
    double perturb_factor = 1.0;
    if (particles[i].color == Particle::GREEN) perturb_factor = 0.5;
    if (particles[i].color == Particle::BLUE) perturb_factor = 0.25;
    particles[i].x += dist(e2) * perturb_factor;
    particles[i].y += dist(e2) * perturb_factor;

    int px = (int)std::floor(particles[i].x / size);
    int py = (int)std::floor(particles[i].y / size);

    if (px == thisIndex.x && py == thisIndex.y) {
      next_particles.push_back(particles[i]);
      continue;
    }

    particles[i].x = fmod(particles[i].x, 100.0);
    particles[i].y = fmod(particles[i].y, 100.0);
    if (particles[i].x < 0) particles[i].x += 100.0;
    if (particles[i].y < 0) particles[i].y += 100.0;

    px -= thisIndex.x - 1; py -= thisIndex.y - 1;
    outside[px][py].push_back(i);
  }

  for (int i = 0; i < 9; ++i) {
    int tx = i % 3; int ty = i / 3;
    if (tx == 1 && ty == 1) continue; // ignore self

    std::vector<int> p_idxs = outside[tx][ty];
    std::vector<Particle> ps;
    for (int j : p_idxs) ps.push_back(particles[j]);

    tx = (tx + thisIndex.x - 1) % k;
    ty = (ty + thisIndex.y - 1) % k;
    if (tx < 0) tx += k;
    if (ty < 0) ty += k;

    thisProxy(tx, ty).recv_particle(iter, ps);
  }

  particles = next_particles;
}

void Box::render(liveVizRequestMsg *m)
{
  char *imageBuf = new char[3 * 100 * 100];
  memset(imageBuf, 0, 3 * 100 * 100);

  for (Particle p : particles) {
    int px = (((p.x - x) / size) * 100.0);
    int py = (((p.y - y) / size) * 100.0);
    int idx = (py * 100 + px) * 3;
    imageBuf[idx] = p.color == Particle::RED ? 0xff : 0;
    imageBuf[idx + 1] = p.color == Particle::GREEN ? 0xff : 0;
    imageBuf[idx + 2] = p.color == Particle::BLUE ? 0xff : 0;
  }

  liveVizDeposit(
    m,
    thisIndex.x * 100, thisIndex.y * 100,
    100, 100,
    (const unsigned char *)imageBuf,
    this
    );

  delete[] imageBuf;
}

#include "particles.def.h"
