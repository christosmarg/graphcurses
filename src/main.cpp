#include <ncurses.h>
#include <cmath>
#include <matheval.h>
#include <iostream>
#include <string>

#define XMIN_PLANE -2*M_PI 
#define XMAX_PLANE 2*M_PI
#define YMIN_PLANE -M_PI
#define YMAX_PLANE M_PI

typedef struct
{
	double ymin, ymax;
	double xmin, xmax;
} Plane;

typedef double (*YFunc)(double x);
void *eval = nullptr;
double default_func(double x) {return sin(x);}
double evalf(double x) {return evaluator_evaluate_x(eval, x);}

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

double scale(double val, double omin, double omax, double nmin, double nmax)
{
	double s = (val - omin) / (omax - omin);
	return s * (nmax - nmin) + nmin;
}

void getstep(Plane &plane, double &xstep, double &ystep)
{
	int ymax, xmax;
	getmaxyx(stdscr, ymax, xmax);
	if (xstep) xstep = (plane.xmax - plane.xmin) / (xmax + 1);
	if (ystep) ystep = (plane.ymax - plane.ymin) / (ymax + 1);
}

void draw_axes(Plane &plane)
{
	int ymax, xmax;
	getmaxyx(stdscr, ymax, xmax);
	double x0 = scale(0, plane.xmin, plane.xmax, 0, xmax);
	double y0 = scale(0, plane.ymin, plane.ymax, ymax, 0);
	double xstep, ystep;
	getstep(plane, xstep, ystep);

	for (int i = 0; i < xmax; i++)
	{
		double plotx = plane.xmin + xstep * i;
		mvwaddch(stdscr, y0, i, ACS_HLINE);
	}

	for (int i = 0; i < ymax; i++)
	{
		double ploty = plane.ymin + ystep * i;
		mvwaddch(stdscr, i, x0, ACS_VLINE);
	}

	refresh();
}

void plot(Plane &plane, double x, double y)
{
	int ymax, xmax;
	getmaxyx(stdscr, ymax, xmax);
	double xp = scale(x, plane.xmin, plane.xmax, 0, xmax);
	double yp = scale(y, plane.ymin, plane.ymax, ymax, 0);
	mvwaddch(stdscr, yp, xp, '.');
}

void draw_graph(Plane &plane, YFunc yfunc)
{
	double xstep;
	double ystep;
	getstep(plane, xstep, ystep);
	attron(COLOR_PAIR(2));
	for (double x = plane.xmin; x <= plane.xmax; x += xstep)
	{
		double y = yfunc(x);
		plot(plane, x, y);
	}
	attroff(COLOR_PAIR(2));
}

void handle_zoom(int key, Plane &plane)
{
	// improve
	if (key == '+')
	{
		plane.xmin += 2.0;
		plane.xmax -= 2.0;
		plane.ymin += 2.0;
		plane.ymax -= 2.0;
	}
	else if (key == '-')
	{
		plane.xmin -= 2.0;
		plane.xmax += 2.0;
		plane.ymin -= 2.0;
		plane.ymax += 2.0;
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
	double xshift = 0, yshift = 0;

	switch (key)
	{
		case 'k': case KEY_UP:    yshift = 1; break;
		case 'j': case KEY_DOWN:  yshift = -1; break;
		case 'h': case KEY_LEFT:  xshift = -1; break;
		case 'l': case KEY_RIGHT: xshift = 1; break;
		case '+': case '-': case 'r': handle_zoom(key, plane); break;
	}

	xshift *= (plane.xmax - plane.xmin) / 16;
	yshift *= (plane.ymax - plane.ymin) / 16;
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
		if (std::string(argv[1]) == "y=") argv[1] += 2;
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
