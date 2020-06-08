
#ifndef _PARTICLES_H_
#define _PARTICLES_H_

struct Particle
{
  double x, y;
  typedef enum { RED, GREEN, BLUE } color_t;
  color_t color;

  Particle() : x(0), y(0), color(RED) {}
  Particle(double sx, double sy, color_t c) : x(sx), y(sy), color(c) {}

  void pup(PUP::er &p)
    {
      p | x; p | y; p | color;
    }
};

#endif /* _PARTICLES_H_ */
