#include "plane.h"

static float plane_scale(float, float, float, float, float);
static void  get_step(const struct Plane *, float *, float *);
static void  graph_plot(const struct Plane *, float, float);

void
plane_init(struct Plane *p)
{
    p->xmin   = XMIN_PLANE;
    p->xmax   = XMAX_PLANE;
    p->ymin   = YMIN_PLANE;
    p->ymax   = YMAX_PLANE;
    p->xscale = XSCALE_PLANE;
    p->yscale = YSCALE_PLANE;
    p->xmaxs  = getmaxx(stdscr);
    p->ymaxs  = getmaxy(stdscr);
}

void
plane_shift(struct Plane *p, float xshift, float yshift)
{
    xshift  *= (p->xmax - p->xmin) / 16.0f;
    yshift  *= (p->ymax - p->ymin) / 16.0f;
    p->xmin += xshift;
    p->xmax += xshift;
    p->ymin += yshift;
    p->ymax += yshift;
}

float
plane_scale(float val, float omin, float omax, float nmin, float nmax)
{
    float  s = (val - omin) / (omax - omin);
    return s * (nmax - nmin) + nmin;
}

void
zoom_restore(struct Plane *p)
{
    p->xmin   = XMIN_PLANE;
    p->xmax   = XMAX_PLANE;
    p->ymin   = YMIN_PLANE;
    p->ymax   = YMAX_PLANE;
    p->xscale = XSCALE_PLANE;
    p->yscale = YSCALE_PLANE;
}

void
zoom_handle(struct Plane *p, float factor)
{
    float xctr = (p->xmin + p->ymax) / 2.0f;
    float yctr = (p->ymin + p->ymax) / 2.0f;
    p->xmin = plane_scale(factor, 1.0f, 0.0f, p->xmin, xctr);
    p->xmax = plane_scale(factor, 1.0f, 0.0f, p->xmax, xctr);
    p->ymin = plane_scale(factor, 1.0f, 0.0f, p->ymin, yctr);
    p->ymax = plane_scale(factor, 1.0f, 0.0f, p->ymax, yctr);  
}

void
get_step(const struct Plane *p, float *xstep, float *ystep)
{
    *xstep = (p->xmax - p->xmin) / (p->xmaxs + 1.0f);
    *ystep = (p->ymax - p->ymin) / (p->ymaxs + 1.0f);
}

void
axes_draw(const struct Plane *p)
{
    int i;
    float x0 = plane_scale(0.0f, p->xmin, p->xmax, 0.0f, p->xmaxs);
    float y0 = plane_scale(0.0f, p->ymin, p->ymax, p->ymaxs, 0.0f);
    float xstep, ystep;
    get_step(p, &xstep, &ystep);
    for (i = 0; i < p->xmaxs; i++) {
        float plotx = p->xmin + xstep * i;
        int tick = fabs(fmod(plotx, p->xscale)) < xstep;
        mvaddch(y0, i, tick ? ACS_PLUS : ACS_HLINE);
    }
    for (i = 0; i < p->ymaxs; i++) {
        float ploty = p->ymin + ystep * i;
        int tick = fabs(fmod(ploty, p->yscale)) < ystep;
        mvaddch(i, x0, tick ? ACS_PLUS : ACS_VLINE);
    }
    mvaddch(y0, x0, ACS_PLUS);
}

void
graph_draw(const struct Plane *p)
{
    float x, xstep, ystep;
    get_step(p, &xstep, &ystep);
    attron(COLOR_PAIR(2));
    for (x = p->xmin; x <= p->xmax; x += xstep) {
        float y = p->yfunc(x);
        graph_plot(p, x, y);
    }
    attroff(COLOR_PAIR(2));
}

void
graph_plot(const struct Plane *p, float x, float y)
{
    float xp = plane_scale(x, p->xmin, p->xmax, 0.0f, p->xmaxs);
    float yp = plane_scale(y, p->ymin, p->ymax, p->ymaxs, 0.0f);
    mvaddch(yp, xp, '.');
}
