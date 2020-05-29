#ifndef PLANE_H
#define PLANE_H

#include <math.h>
#include <matheval.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

#define XMIN_PLANE  -2.0f*M_PI;
#define XMAX_PLANE   2.0f*M_PI;
#define YMIN_PLANE  -M_PI;
#define YMAX_PLANE   M_PI;
#define XSCALE_PLANE 1.0f;
#define YSCALE_PLANE 1.0f;

typedef struct {
    float (*yfunc)(float x);
    float ymin, ymax;
    float xmin, xmax;
    float xscale, yscale;
    int ymaxs, xmaxs;
} Plane;

extern Plane p;

void plane_init(Plane *p);
void restore_zoom(Plane *p);
void draw_axes(const Plane *p);
void draw_graph(const Plane *p);
float scale(float val, float omin, float omax, float nmin, float nmax);
void getstep(const Plane *p, float *xstep, float *ystep);
void plot(const Plane *p, float x, float y);
void handle_zoom(Plane *p, float factor);
void shift(Plane *p, float xshift, float yshift);

#endif /* PLANE_H */
