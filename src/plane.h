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

struct Plane {
    float (*yfunc)(float);
    float  ymin, ymax;
    float  xmin, xmax;
    float  xscale, yscale;
    int    ymaxs, xmaxs;
};

extern struct Plane p;

extern void  plane_init(struct Plane *);
extern void  plane_shift(struct Plane *, float, float);
extern void  zoom_restore(struct Plane *);
extern void  zoom_handle(struct Plane *, float);
extern void  axes_draw(const struct Plane *);
extern void  graph_draw(const struct Plane *);

#endif /* PLANE_H */
