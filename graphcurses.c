/* See LICENSE file for copyright and license details. */
#include <err.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curses.h>
#include <matheval.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif /* M_PI */
#ifndef SIGWINCH
#define SIGWINCH 28
#endif /* SIGWINCH */
#define SHIFT_STEP 1.0f
#define BUFSIZE 256

#define YMAX (getmaxy(stdscr))
#define XMAX (getmaxx(stdscr))
#define CENTER(x, y) (((x) >> 1) - ((y) >> 1))
#define PLANE_SCALE(val, omin, omax, nmin, nmax)			      \
	((((val) - (omin)) / ((omax) - (omin))) * ((nmax) - (nmin)) + (nmin))
#define PLANE_XSTEP(p, xstep)						      \
	xstep = (p->xmax - p->xmin) / (p->xmaxs + 1.0f);		       
#define PLANE_YSTEP(p, ystep)						      \
	ystep = (p->xmax - p->ymin) / (p->ymaxs + 1.0f);
#define ARRLEN(x) (sizeof(x) / sizeof(x[0]))

struct plane {
	float (*f)(float);
	void *df;
	float ymin;
	float ymax;
	float xmin;
	float xmax;
	float xscale;
	float yscale;
	int ymaxs;
	int xmaxs;
	int derivshow;
};

enum {
	C_FG = 1,
	C_F,
	C_DF,
};

static void cursesinit(void);
static void exprvalidate(void);
static float expreval(float);
static void planeinit(void);
static void planeshift(float, float);
static void zoomrestore(void);
static void zoomhandle(float);
static void axesdraw(void);
static void graphdraw(void);
static void graphplot(float, float);
static void menuopts(void);
static void sighandler(int);
static void cleanup(void);

static struct plane *p;
static void *f = NULL;
static int colors[] = {
	[C_FG] = COLOR_BLUE,
	[C_F] = COLOR_YELLOW,
	[C_DF] = COLOR_MAGENTA,
};

static void
cursesinit(void)
{
	struct sigaction sa;
	int i;

	if (!initscr())
		errx(1, "initscr");
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, 1);

	start_color();
	use_default_colors();
	for (i = 1; i < ARRLEN(colors); i++)
		(void)init_pair(i, colors[i], -1);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sighandler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGWINCH, &sa, NULL);
}

static void
exprvalidate(void)
{
	char *buf;

	if ((buf = malloc(BUFSIZE)) == NULL)
		err(1, "malloc");
	attron(COLOR_PAIR(C_FG));
	for (;;) {
		move(0, 0);
		clrtoeol();
		printw("f(x) = ");
		echo();
		refresh();
		if (getnstr(buf, BUFSIZE) == ERR)
			continue;
		zoomrestore();
		refresh();
		noecho();
		if (!(f = evaluator_create(buf)))
			printw("Error in expression! Try again");
		else
			break;
		refresh();
	}
	attroff(COLOR_PAIR(C_FG));
	p->df = evaluator_derivative_x(f);
	free(buf);
}

static float
expreval(float x)
{
	return evaluator_evaluate_x(f, x);
}

static void
planeinit(void)
{
	if ((p = malloc(sizeof(struct plane))) == NULL)
		err(1, "malloc");
	p->xmaxs = XMAX;
	p->ymaxs = YMAX;
	p->derivshow = 0;
	p->f = expreval;
	zoomrestore();
}

static void
planeshift(float xshift, float yshift)
{
	xshift *= (p->xmax - p->xmin) / 16.0f;
	yshift *= (p->ymax - p->ymin) / 16.0f;
	p->xmin += xshift;
	p->xmax += xshift;
	p->ymin += yshift;
	p->ymax += yshift;
}

static void
zoomrestore(void)
{
	p->xmin = -2.0f * M_PI;
	p->xmax = 2.0f * M_PI;
	p->ymin = -M_PI;
	p->ymax = M_PI;
	p->xscale = 1.0f;
	p->yscale = 1.0f;
}

static void
zoomhandle(float factor)
{
	float xctr = (p->xmin + p->ymax) / 2.0f;
	float yctr = (p->ymin + p->ymax) / 2.0f;

	p->xmin = PLANE_SCALE(factor, 1.0f, 0.0f, p->xmin, xctr);
	p->xmax = PLANE_SCALE(factor, 1.0f, 0.0f, p->xmax, xctr);
	p->ymin = PLANE_SCALE(factor, 1.0f, 0.0f, p->ymin, yctr);
	p->ymax = PLANE_SCALE(factor, 1.0f, 0.0f, p->ymax, yctr);  
}

static void
axesdraw(void)
{
	float x0, y0, xstep, ystep, plotx, ploty, i;
	int tick;

	x0 = PLANE_SCALE(0.0f, p->xmin, p->xmax, 0.0f, p->xmaxs);
	y0 = PLANE_SCALE(0.0f, p->ymin, p->ymax, p->ymaxs, 0.0f);
	PLANE_XSTEP(p, xstep);
	PLANE_YSTEP(p, ystep);

	for (i = 0.0f; i < p->xmaxs; i += xstep) {
		plotx = p->xmin + xstep * i;
		tick = fabs(fmod(plotx, p->xscale)) < xstep;
		mvaddch(y0, i, tick ? ACS_PLUS : ACS_HLINE);
	}
	for (i = 0.0f; i < p->ymaxs; i += ystep) {
		ploty = p->ymin + ystep * i;
		tick = fabs(fmod(ploty, p->yscale)) < ystep;
		mvaddch(i, x0, tick ? ACS_PLUS : ACS_VLINE);
	}
	mvaddch(y0, x0, ACS_PLUS);
}

static void
graphdraw(void)
{
	float x, y, dy, xstep;

	PLANE_XSTEP(p, xstep);
	for (x = p->xmin; x <= p->xmax; x += xstep) {
		y = p->f(x);
		attron(COLOR_PAIR(C_F));
		graphplot(x, y);
		if (p->derivshow) {
			dy = evaluator_evaluate_x(p->df, x);
			attron(COLOR_PAIR(C_DF));
			graphplot(x, dy);
		}
	}
	attroff(COLOR_PAIR(C_F) | COLOR_PAIR(C_DF));
}

static void
graphplot(float x, float y)
{
	float xp = PLANE_SCALE(x, p->xmin, p->xmax, 0.0f, p->xmaxs);
	float yp = PLANE_SCALE(y, p->ymin, p->ymax, p->ymaxs, 0.0f);

	mvaddch(yp, xp, '.');
}

static void
menuopts(void)
{
	WINDOW *opts;
	int w, h, wy, wx;

	w = 33;
	h = 14;
	wy = CENTER(YMAX, h);
	wx = CENTER(XMAX, w);
	if ((opts = newwin(h, w, wy, wx)) == NULL)
		errx(1, "newwin");
	werase(opts);
	wattron(opts, COLOR_PAIR(C_FG));
	box(opts, 0, 0);

	/* fill menu */
	mvwprintw(opts, 1,  1, "q              Quit");
	mvwprintw(opts, 2,  1, "Up/k           Move up");
	mvwprintw(opts, 3,  1, "Down/j         Move down");
	mvwprintw(opts, 4,  1, "Left/h         Move left");
	mvwprintw(opts, 5,  1, "Right/l        Move right");
	mvwprintw(opts, 6,  1, "d              Show derivative");
	mvwprintw(opts, 7,  1, "f              New function");
	mvwprintw(opts, 8,  1, "r              Restore zoom");
	mvwprintw(opts, 9,  1, "+              Zoom in");
	mvwprintw(opts, 10, 1, "-              Zoom out");
	mvwprintw(opts, 12, 1, "Press any key to quit the menu");

	wrefresh(opts);
	wattroff(opts, COLOR_PAIR(C_FG));
	(void)wgetch(opts);
	werase(opts);
	wrefresh(opts);
	delwin(opts);
}

static void
sighandler(int sig)
{
	switch (sig) {
	case SIGINT: /* FALLTHROUGH */
	case SIGTERM:
		cleanup();
		exit(0);
	case SIGWINCH:
		/* TODO: lol... */
		break;
	}
}

static void
cleanup(void)
{
	endwin();
	evaluator_destroy(f);
	free(p);
}

int
main(int argc, char *argv[])
{
	int key = 0;

	cursesinit();
	planeinit();
	exprvalidate();

	for (; key != 'q'; key = getch()) {
		switch (key) {
		case KEY_UP:	/* FALLTHROUGH */
		case 'k':
			planeshift(0.0f,  SHIFT_STEP);
			break;
		case KEY_DOWN:	/* FALLTHROUGH */
		case 'j':
			planeshift(0.0f, -SHIFT_STEP);
			break;
		case KEY_LEFT:	/* FALLTHROUGH */
		case 'h':
			planeshift(-SHIFT_STEP, 0.0f);
			break;
		case KEY_RIGHT:	/* FALLTHROUGH */
		case 'l':
			planeshift(SHIFT_STEP, 0.0f);
			break;
		case '+':
			zoomhandle(1.0f / 1.05f);
			break;
		case '-':
			zoomhandle(1.05f);
			break;
		case 'd':
			p->derivshow ^= 1;
			break;
		case 'r':
			zoomrestore();
			break;
		case 'f':
			exprvalidate();
			break;
		case 'c':
			menuopts();
			break;
		}
		erase();
		attron(COLOR_PAIR(C_FG) | A_REVERSE | A_BOLD);
		mvprintw(0, 0, "f(x) = %s", evaluator_get_string(f));
		/* TODO: print controls opt */
		if (p->derivshow)
			mvprintw(1, 0, "f'(x) = %s", evaluator_get_string(p->df));
		attroff(A_REVERSE | A_BOLD);
		axesdraw();
		attroff(COLOR_PAIR(C_FG));
		graphdraw();
		refresh();
	}
	cleanup();

	return 0;
}
