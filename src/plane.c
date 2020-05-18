#include "plane.h"

void
plane_init(Plane *p)
{
	p->xmin		= XMIN_PLANE;
	p->xmax		= XMAX_PLANE;
	p->ymin		= YMIN_PLANE;
	p->ymax		= YMAX_PLANE;
	p->xscale	= XSCALE_PLANE;
	p->yscale	= YSCALE_PLANE;
	p->xmaxs	= getmaxx(stdscr);
	p->ymaxs	= getmaxy(stdscr);
}

void
restore_zoom(Plane *p)
{
	p->xmin		= XMIN_PLANE;
	p->xmax		= XMAX_PLANE;
	p->ymin		= YMIN_PLANE;
	p->ymax		= YMAX_PLANE;
	p->xscale	= XSCALE_PLANE;
	p->yscale	= YSCALE_PLANE;
}

void
draw_axes(const Plane *p)
{
	int i;
	float x0 = scale(0.0f, p->xmin, p->xmax, 0.0f, p->xmaxs);
	float y0 = scale(0.0f, p->ymin, p->ymax, p->ymaxs, 0.0f);
	float xstep, ystep;
	getstep(p, &xstep, &ystep);
	for (i = 0; i < p->xmaxs; i++)
	{
		float plotx = p->xmin + xstep * i;
		int tick = fabs(fmod(plotx, p->xscale)) < xstep;
		mvaddch(y0, i, tick ? ACS_PLUS : ACS_HLINE);
	}
	for (i = 0; i < p->ymaxs; i++)
	{
		float ploty = p->ymin + ystep * i;
		int tick = fabs(fmod(ploty, p->yscale)) < ystep;
		mvaddch(i, x0, tick ? ACS_PLUS : ACS_VLINE);
	}
	mvaddch(y0, x0, ACS_PLUS);
}

void
draw_graph(const Plane *p)
{
	float x;
	float xstep;
	float ystep;
	getstep(p, &xstep, &ystep);
	attron(COLOR_PAIR(2));
	for (x = p->xmin; x <= p->xmax; x += xstep)
	{
		float y = p->yfunc(x);
		plot(p, x, y);
	}
	attroff(COLOR_PAIR(2));
}

float
scale(float val, float omin, float omax, float nmin, float nmax)
{
	float s = (val - omin) / (omax - omin);
	return s * (nmax - nmin) + nmin;
}

void
getstep(const Plane *p, float *xstep, float *ystep)
{
	if (*xstep) *xstep = (p->xmax - p->xmin) / (p->xmaxs + 1.0f);
	if (*ystep) *ystep = (p->ymax - p->ymin) / (p->ymaxs + 1.0f);
}

void
plot(const Plane *p, float x, float y)
{
	float xp = scale(x, p->xmin, p->xmax, 0.0f, p->xmaxs);
	float yp = scale(y, p->ymin, p->ymax, p->ymaxs, 0.0f);
	mvaddch(yp, xp, '.');
}


void
handle_zoom(Plane *p, float factor)
{
	float centerX = (p->xmin + p->ymax) / 2.0f;
	float centerY = (p->ymin + p->ymax) / 2.0f;
	p->xmin = scale(factor, 1.0f, 0.0f, p->xmin, centerX);
	p->xmax = scale(factor, 1.0f, 0.0f, p->xmax, centerX);
	p->ymin = scale(factor, 1.0f, 0.0f, p->ymin, centerY);
	p->ymax = scale(factor, 1.0f, 0.0f, p->ymax, centerY);	
}

void
shift(Plane *p, float xshift, float yshift)
{
	xshift *= (p->xmax - p->xmin) / 16.0f;
	yshift *= (p->ymax - p->ymin) / 16.0f;
	p->xmin += xshift;
	p->xmax += xshift;
	p->ymin += yshift;
	p->ymax += yshift;
}
