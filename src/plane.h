#ifndef PLANE_H
#define PLANE_H

#include <cmath>
#include <functional>
#include <iostream>
#include <matheval.h>
#include <ncurses.h>

class Plane
{
	private:
		const float XMIN_PLANE =	-2.0f*M_PI;
		const float XMAX_PLANE =	 2.0f*M_PI;
		const float YMIN_PLANE =	-M_PI;
		const float YMAX_PLANE =	 M_PI;
		const float XSCALE_PLANE =	 1.0f;
		const float YSCALE_PLANE =	 1.0f;

		float ymin, ymax;
		float xmin, xmax;
		float xscale, yscale;
		int ymaxs, xmaxs;

	public:
		Plane();

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
