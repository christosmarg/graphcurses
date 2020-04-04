#include "plane.h"

void Plane::restore_zoom()
{
	xmin = XMIN_PLANE;
	xmax = XMAX_PLANE;
	ymin = YMIN_PLANE;
	ymax = YMAX_PLANE;
	xscale = XSCALE_PLANE;
	yscale = YSCALE_PLANE;
}

void Plane::draw_axes()
{
	float x0 = scale(0.0f, xmin, xmax, 0.0f, xmaxs);
	float y0 = scale(0.0f, ymin, ymax, ymaxs, 0.0f);
	float xstep, ystep;
	getstep(xstep, ystep);

	for (int i = 0; i < xmaxs; i++)
	{
		float plotx = xmin + xstep * i;
		int tick = fabs(fmod(plotx, xscale)) < xstep;
		mvaddch(y0, i, tick ? ACS_PLUS : ACS_HLINE);
		// add numbering
	}

	for (int i = 0; i < ymaxs; i++)
	{
		float ploty = ymin + ystep * i;
		int tick = fabs(fmod(ploty, yscale)) < ystep;
		mvaddch(i, x0, tick ? ACS_PLUS : ACS_VLINE);
		// add numbering
	}
}

void Plane::draw_graph(const std::function<float(float)>& yfunc)
{
	float xstep;
	float ystep;
	getstep(xstep, ystep);
	attron(COLOR_PAIR(2));
	for (float x = xmin; x <= xmax; x += xstep)
	{
		float y = yfunc(x);
		plot(x, y);
	}
	attroff(COLOR_PAIR(2));
}

float Plane::scale(float val, float omin, float omax, float nmin, float nmax)
{
	float s = (val - omin) / (omax - omin);
	return s * (nmax - nmin) + nmin;
}

void Plane::getstep(float &xstep, float &ystep)
{
	if (xstep) xstep = (xmax - xmin) / (xmaxs + 1.0f);
	if (ystep) ystep = (ymax - ymin) / (ymaxs + 1.0f);
}

void Plane::plot(float x, float y)
{
	float xp = scale(x, xmin, xmax, 0.0f, xmaxs);
	float yp = scale(y, ymin, ymax, ymaxs, 0.0f);
	mvaddch(yp, xp, '.');
}


void Plane::handle_zoom(float factor)
{
	float centerX = (xmin + ymax) / 2.0f;
	float centerY = (ymin + ymax) / 2.0f;
	xmin = scale(factor, 1.0f, 0.0f, xmin, centerX);
	xmax = scale(factor, 1.0f, 0.0f, xmax, centerX);
	ymin = scale(factor, 1.0f, 0.0f, ymin, centerY);
	ymax = scale(factor, 1.0f, 0.0f, ymax, centerY);	
}

void Plane::shift(float xshift, float yshift)
{
	xshift *= (xmax - xmin) / 16.0f;
	yshift *= (ymax - ymin) / 16.0f;
	xmin += xshift;
	xmax += xshift;
	ymin += yshift;
	ymax += yshift;
}
