#ifndef PLANE_H
#define PLANE_H

#include <math.h>
#include <matheval.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

#define XMIN_PLANE      (-2.0f * M_PI)
#define XMAX_PLANE      ( 2.0f * M_PI)
#define YMIN_PLANE      -M_PI
#define YMAX_PLANE       M_PI
#define XSCALE_PLANE     1.0f
#define YSCALE_PLANE     1.0f
#define SHIFT_STEP       1.0f
#define ZOOM_IN_FACTOR  (1.0f / 1.05f)
#define ZOOM_OUT_FACTOR  1.05f

typedef struct {
    float (*yfunc)(float x);
    float ymin, ymax;
    float xmin, xmax;
    float xscale, yscale;
    int   ymaxs, xmaxs;
} Plane;

extern Plane p;

extern void  plane_init(Plane *p);
extern float plane_scale(float val, float omin, float omax, float nmin, float nmax);
extern void  plane_shift(Plane *p, float xshift, float yshift);
extern void  zoom_restore(Plane *p);
extern void  zoom_handle(Plane *p, float factor);
extern void  get_step(const Plane *p, float *xstep, float *ystep);
extern void  axes_draw(const Plane *p);
extern void  graph_draw(const Plane *p);
extern void  graph_plot(const Plane *p, float x, float y);

#endif /* PLANE_H */
