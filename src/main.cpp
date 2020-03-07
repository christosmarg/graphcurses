#include <ncurses.h>
#include <cmath>
#include <matheval.h>
#include <iostream>
#include <string>

#define XMIN_PLANE -2.0f*M_PI 
#define XMAX_PLANE 2.0f*M_PI
#define YMIN_PLANE -M_PI
#define YMAX_PLANE M_PI

typedef struct
{
	float ymin, ymax;
	float xmin, xmax;
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
		mvwaddch(stdscr, y0, i, ACS_HLINE);
	}

	for (int i = 0; i < ymax; i++)
	{
		float ploty = plane.ymin + ystep * i;
		mvwaddch(stdscr, i, x0, ACS_VLINE);
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

void handle_zoom(int key, Plane &plane)
{
	// improve
	if (key == '+')
	{
		plane.xmin += 1.5f;
		plane.xmax -= 1.5f;
		plane.ymin += 1.5f;
		plane.ymax -= 1.5f;
	}
	else if (key == '-')
	{
		plane.xmin -= 1.5f;
		plane.xmax += 1.5f;
		plane.ymin -= 1.5f;
		plane.ymax += 1.5f;
	}
	else if (key == 'r')
	{
		plane.xmin = XMIN_PLANE;
		plane.xmax = XMAX_PLANE;
		plane.ymin = YMIN_PLANE;
		plane.ymax = YMAX_PLANE;
	}

}

void handle_key(int key, Plane &plane)
{
	float xshift = 0.0f, yshift = 0.0f;

	switch (key)
	{
		case 'k': case 'w': case KEY_UP:    yshift = 1; break;
		case 'j': case 's': case KEY_DOWN:  yshift = -1; break;
		case 'h': case 'a': case KEY_LEFT:  xshift = -1; break;
		case 'l': case 'd': case KEY_RIGHT: xshift = 1; break;
		case '+': case '-': case 'r': handle_zoom(key, plane); break;
	}

	xshift *= (plane.xmax - plane.xmin) / 16.0f;
	yshift *= (plane.ymax - plane.ymin) / 16.0f;
	plane.xmin += xshift;
	plane.xmax += xshift;
	plane.ymin += yshift;
	plane.ymax += yshift;
}

int main(int argc, char **argv)
{
	Plane plane;
	plane.xmin = XMIN_PLANE;
	plane.xmax = XMAX_PLANE;
	plane.ymin = YMIN_PLANE;
	plane.ymax = YMAX_PLANE;

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
		handle_key(key, plane);
		erase();
		attron(COLOR_PAIR(1));
		draw_axes(plane);
		attroff(COLOR_PAIR(1));

		draw_graph(plane, yfunc);
		refresh();
		key = getch();
	}
	
	endwin();

	return 0;
}
