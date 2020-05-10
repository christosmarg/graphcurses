#include "plane.h"

static void *f = nullptr;

static void init_curses();
static void getfunc(char *buffer, Plane& plane);
static void validate_expression(Plane& plane);
static void handle_key(int key, Plane& plane);

int
main(int argc, char **argv)
{
	init_curses();
	Plane plane;
	plane.restore_zoom();
	validate_expression(plane);
	std::function<float(float)> yfunc =
		[](float x){return evaluator_evaluate_x(f, x);};
	int key = 0;
	
	while (key != 'q')
	{
		attron(COLOR_PAIR(1));
		handle_key(key, plane);
		erase();
		attron(A_REVERSE);
		attron(A_BOLD);
		mvprintw(0, 0, "f(x) = %s", evaluator_get_string(f));
		attroff(A_REVERSE);
		attroff(A_BOLD);
		plane.draw_axes();
		attroff(COLOR_PAIR(1));

		plane.draw_graph(yfunc);
		refresh();
		key = getch();
	}
	
	endwin();
	evaluator_destroy(f);	

	return 0;
}

static void
init_curses()
{
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, true);
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_YELLOW, COLOR_BLACK);
}

static void
getfunc(char *buffer, Plane& plane)
{
	move(0, 0);
	clrtoeol();
	printw("f(x) = ");
	echo();
	refresh();
	getnstr(buffer, 256);
	plane.restore_zoom();
	refresh();
	noecho();
}

static void
validate_expression(Plane& plane)
{
	char *buffer = new char[256];
	getfunc(buffer, plane);
	while (!(f = evaluator_create(buffer)))
	{
		printw("Error in expression! Try again");
		getfunc(buffer, plane);
		refresh();
	}
	delete[] buffer;
}

static void
handle_key(int key, Plane& plane)
{
	switch (key)
	{
		case 'k': case KEY_UP:    plane.shift(0.0f, 1.0f); break;
		case 'j': case KEY_DOWN:  plane.shift(0.0f, -1.0f); break;
		case 'h': case KEY_LEFT:  plane.shift(-1.0f, 0.0f); break;
		case 'l': case KEY_RIGHT: plane.shift(1.0f, 0.0f); break;
		case '+': plane.handle_zoom(1.0f/1.05f); break;
		case '-': plane.handle_zoom(1.05f); break;
		case 'r': plane.restore_zoom(); break;
		case 'f': validate_expression(plane); break;
	}
}
