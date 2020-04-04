#ifndef PLANE_H
#define PLANE_H

#include <ncurses.h>
#include <iostream>
#include <functional>
#include <cmath>
#include <matheval.h>

#define XMIN_PLANE -2.0f*M_PI 
#define XMAX_PLANE 2.0f*M_PI
#define YMIN_PLANE -M_PI
#define YMAX_PLANE M_PI
#define XSCALE_PLANE 1.0f
#define YSCALE_PLANE 1.0f

class Plane
{
	private:
		float ymin, ymax;
		float xmin, xmax;
		float xscale, yscale;
		int ymaxs, xmaxs;

	public:
		Plane()
			:ymin(YMIN_PLANE), ymax(YMAX_PLANE), xmin(XMIN_PLANE), xmax(XMAX_PLANE),
			xscale(XSCALE_PLANE), yscale(YSCALE_PLANE), ymaxs(getmaxy(stdscr)), xmaxs(getmaxx(stdscr)) {}

		void restore_zoom();
		void draw_axes();
		void draw_graph(const std::function<float(float)>& yfunc);
		float scale(float val, float omin, float omax, float nmin, float nmax);
		void getstep(float &xstep, float &ystep);
		void plot(float x, float y);
		void handle_zoom(float factor);
		void shift(float xshift = 0.0f, float yshift = 0.0f);
};

#endif /* PLANE_H */
