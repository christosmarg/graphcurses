#include <ncurses.h>
#include <cmath>
#include <matheval.h>
#include <iostream>
#include <string>

#define XMIN_PLANE -2.0f*M_PI 
#define XMAX_PLANE 2.0f*M_PI
#define YMIN_PLANE -M_PI
#define YMAX_PLANE M_PI
#define XSCALE_PLANE 1.0f
#define YSCALE_PLANE 1.0f

typedef struct
{
	float ymin, ymax;
	float xmin, xmax;
	float xscale, yscale;
} Plane;

typedef float (*YFunc)(float x);
void *eval = nullptr;
float default_func(float x) {return sin(x);}
float evalf(float x) {return evaluator_evaluate_x(eval, x);}

void init_curses()
{
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, true);
	start_color();
	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	init_pair(2, COLOR_YELLOW, COLOR_BLACK);
}

float scale(float val, float omin, float omax, float nmin, float nmax)
{
	float s = (val - omin) / (omax - omin);
	return s * (nmax - nmin) + nmin;
}

void getstep(Plane &plane, float &xstep, float &ystep)
{
	int ymax, xmax;
	getmaxyx(stdscr, ymax, xmax);
	if (xstep) xstep = (plane.xmax - plane.xmin) / (xmax + 1.0f);
	if (ystep) ystep = (plane.ymax - plane.ymin) / (ymax + 1.0f);
}

void draw_axes(Plane &plane)
{
	int ymax, xmax;
	getmaxyx(stdscr, ymax, xmax);
	float x0 = scale(0.0f, plane.xmin, plane.xmax, 0.0f, xmax);
	float y0 = scale(0.0f, plane.ymin, plane.ymax, ymax, 0.0f);
	float xstep, ystep;
	getstep(plane, xstep, ystep);

	for (int i = 0; i < xmax; i++)
	{
		float plotx = plane.xmin + xstep * i;
		int tick = fabs(fmod(plotx, plane.xscale)) < xstep;
		mvwaddch(stdscr, y0, i, tick ? ACS_PLUS : ACS_HLINE);
		// add numbering
	}

	for (int i = 0; i < ymax; i++)
	{
		float ploty = plane.ymin + ystep * i;
		int tick = fabs(fmod(ploty, plane.yscale)) < ystep;
		mvwaddch(stdscr, i, x0, tick ? ACS_PLUS : ACS_VLINE);
		// add numbering
	}

	refresh();
}

void plot(Plane &plane, float x, float y)
{
	int ymax, xmax;
	getmaxyx(stdscr, ymax, xmax);
	float xp = scale(x, plane.xmin, plane.xmax, 0.0f, xmax);
	float yp = scale(y, plane.ymin, plane.ymax, ymax, 0.0f);
	mvwaddch(stdscr, yp, xp, '.');
}

void draw_graph(Plane &plane, YFunc yfunc)
{
	float xstep;
	float ystep;
	getstep(plane, xstep, ystep);
	attron(COLOR_PAIR(2));
	for (float x = plane.xmin; x <= plane.xmax; x += xstep)
	{
		float y = yfunc(x);
		plot(plane, x, y);
	}
	attroff(COLOR_PAIR(2));
}

void handle_zoom(Plane &plane, float factor)
{
	float centerX = (plane.xmin + plane.ymax) / 2;
	float centerY = (plane.ymin + plane.ymax) / 2;
	plane.xmin = scale(factor, 1.0f, 0.0f, plane.xmin, centerX);
	plane.xmax = scale(factor, 1.0f, 0.0f, plane.xmax, centerX);
	plane.ymin = scale(factor, 1.0f, 0.0f, plane.ymin, centerY);
	plane.ymax = scale(factor, 1.0f, 0.0f, plane.ymax, centerY);	
}

void restore_zoom(Plane &plane)
{
	plane.xmin = XMIN_PLANE;
	plane.xmax = XMAX_PLANE;
	plane.ymin = YMIN_PLANE;
	plane.ymax = YMAX_PLANE;
	plane.xscale = XSCALE_PLANE;
	plane.yscale = YSCALE_PLANE;
}

void shift(Plane &plane, float xshift = 0.0f, float yshift = 0.0f)
{
	xshift *= (plane.xmax - plane.xmin) / 16.0f;
	yshift *= (plane.ymax - plane.ymin) / 16.0f;
	plane.xmin += xshift;
	plane.xmax += xshift;
	plane.ymin += yshift;
	plane.ymax += yshift;
}

void handle_key(int key, Plane &plane)
{
	switch (key)
	{
		case 'k': case 'w': case KEY_UP:    shift(plane, 0.0f, 1.0f); break;
		case 'j': case 's': case KEY_DOWN:  shift(plane, 0.0f, -1.0f); break;
		case 'h': case 'a': case KEY_LEFT:  shift(plane, -1.0f, 0.0f); break;
		case 'l': case 'd': case KEY_RIGHT:	shift(plane, 1.0f, 0.0f); break;
		case '+': handle_zoom(plane, 1.0f/1.05f); break;
		case '-': handle_zoom(plane, 1.05f); break;
		case 'r': restore_zoom(plane); break;
	}
}

int main(int argc, char **argv)
{
	Plane plane;
	restore_zoom(plane);
	YFunc yfunc = default_func;
	int key = 0;

	if (argc > 1)
	{
		eval = evaluator_create(argv[1]);
		if (!eval)
		{
			std::cout << "Error in expression!" << std::endl;
			return -1;
		}
		yfunc = evalf;
	}

	init_curses();
	while (key != 'q')
	{
		erase();
		attron(COLOR_PAIR(1));
		handle_key(key, plane);
		draw_axes(plane);
		attroff(COLOR_PAIR(1));

		draw_graph(plane, yfunc);
		refresh();
		key = getch();
	}
	
	endwin();

	return 0;
}
