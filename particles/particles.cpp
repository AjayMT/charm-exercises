
#include "liveViz.h"
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
  double x, y;
  int iter = 0;
  int sub_iter = 0;

  std::vector<Particle> particles;

  void update_particles();
  void generate_particles(Particle::color_t c, int num);
  void perturb(Particle *p);

public:
  Box(int sn, int sk, CProxy_Main m);
  void render(liveVizRequestMsg *m);
};

Box::Box(int sn, int sk, CProxy_Main m) : n(sn), k(sk), main(m)
{
  x = thisIndex.x;
  y = thisIndex.y;

  std::random_device rd;
  std::mt19937_64 e2(rd());
  std::uniform_real_distribution<> dist(0, 1);

  if (thisIndex.x <= thisIndex.y)
    generate_particles(Particle::GREEN, n);
  if (thisIndex.x >= thisIndex.y)
    generate_particles(Particle::BLUE, n);
  if (
    (thisIndex.x * 8 >= 3 * k && thisIndex.x * 8 <= 5 * k
     && thisIndex.y * 8 >= 3 * k && thisIndex.y * 8 <= 5 * k)
    || thisIndex.x == thisIndex.y
    )
    generate_particles(Particle::RED, n * 2);
}

void Box::generate_particles(Particle::color_t c, int num)
{
  std::random_device rd;
  std::mt19937_64 e2(rd());
  std::uniform_real_distribution<> dist(0, 1);

  for (int i = 0; i < num; ++i) {
    Particle p(x + dist(e2), y + dist(e2), c);
    particles.push_back(p);
  }
}

void Box::perturb(Particle *p)
{
  double velocityFactor = 50.0;
  double deltax = cos(p->y);
  double deltay = cos(p->x);

  double pf = 1;
  if (p->color == Particle::GREEN) pf = 5;
  if (p->color == Particle::BLUE) pf = 2;

  p->x += deltax / (velocityFactor * pf);
  p->y += deltay / (velocityFactor * pf);
}

void Box::update_particles()
{
  std::vector<int> outside[3][3];
  std::vector<Particle> next_particles;

  for (int i = 0; i < particles.size(); ++i) {
    perturb(&particles[i]);

    int px = (int)std::floor(particles[i].x);
    int py = (int)std::floor(particles[i].y);

    if (px == thisIndex.x && py == thisIndex.y) {
      next_particles.push_back(particles[i]);
      continue;
    }

    particles[i].x = fmod(particles[i].x, k);
    particles[i].y = fmod(particles[i].y, k);
    if (particles[i].x < 0) particles[i].x += k;
    if (particles[i].y < 0) particles[i].y += k;

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
  char *imageBuf = new char[3 * 50 * 50];
  memset(imageBuf, 0xff, 3 * 50 * 50);

  for (Particle p : particles) {
    int px = ((p.x - x) * 50.0);
    int py = ((p.y - y) * 50.0);
    int idx = (py * 50 + px) * 3;
    imageBuf[idx] = p.color == Particle::RED ? 0xff : 0;
    imageBuf[idx + 1] = p.color == Particle::GREEN ? 0xff : 0;
    imageBuf[idx + 2] = p.color == Particle::BLUE ? 0xff : 0;
  }

  liveVizDeposit(
    m,
    thisIndex.x * 50, thisIndex.y * 50,
    50, 50,
    (const unsigned char *)imageBuf,
    this
    );

  delete[] imageBuf;
}

#include "particles.def.h"
