#include <math.h>
#include <matheval.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

#define XMIN_PLANE      (-2.0f * M_PI)
#define XMAX_PLANE      ( 2.0f * M_PI)
#define YMIN_PLANE      -M_PI
#define YMAX_PLANE       M_PI
#define XSCALE_PLANE     1.0f
#define YSCALE_PLANE     1.0f
#define SHIFT_STEP       1.0f
#define ZOOM_IN_FACTOR  (1.0f / 1.05f)
#define ZOOM_OUT_FACTOR  1.05f
#define BUFFSIZE         256

#define YMAX()          (getmaxy(stdscr))
#define XMAX()          (getmaxx(stdscr))
#define CENTER(x, y)    ((x) / 2 - (y) / 2)
#define PLANE_SCALE(val, omin, omax, nmin, nmax)                           \
    ((((val) - (omin)) / ((omax) - (omin))) * ((nmax) - (nmin)) + (nmin))
#define PLANE_XSTEP(p, xstep)                                              \
    xstep = (p->xmax - p->xmin) / (p->xmaxs + 1.0f);                       
#define PLANE_YSTEP(p, ystep)                                              \
    ystep = (p->xmax - p->ymin) / (p->ymaxs + 1.0f);

struct Plane {
    float (*f)(float);
    void  *df;
    float  ymin, ymax;
    float  xmin, xmax;
    float  xscale, yscale;
    int    ymaxs, xmaxs;
    int    derivative_show;
};

static void  *f  = NULL;

static void   curses_init(void);
static void   func_get(struct Plane *, char *);
static void   expression_validate(struct Plane *);
static float  expression_evaluate(float);
static void   keys_handle(struct Plane *, int);
static void   plane_init(struct Plane *);
static void   plane_shift(struct Plane *, float, float);
static void   zoom_restore(struct Plane *);
static void   zoom_handle(struct Plane *, float);
static void   axes_draw(const struct Plane *);
static void   graph_draw(const struct Plane *);
static void   graph_plot(const struct Plane *, float, float);
static void   menu_options(void);
static void   menu_fill(struct _win_st *);

void
curses_init(void)
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, 1);
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_CYAN,  COLOR_BLACK);
    init_pair(3, COLOR_RED,   COLOR_BLACK);
}

void
func_get(struct Plane *p, char *buf)
{
    move(0, 0);
    clrtoeol();
    printw("f(x) = ");
    echo();
    refresh();
    getnstr(buf, BUFFSIZE);
    zoom_restore(p);
    refresh();
    noecho();
}

void
expression_validate(struct Plane *p)
{
    char *buf = (char *)malloc(BUFFSIZE + sizeof(char));
    func_get(p, buf);
    while (!(f = evaluator_create(buf))) {
        printw("Error in expression! Try again");
        func_get(p, buf);
        refresh();
    }
    p->df = evaluator_derivative_x(f);
    free(buf);
}

float
expression_evaluate(float x)
{
    return evaluator_evaluate_x(f, x);
}

void
keys_handle(struct Plane *p, int key)
{
    switch (key) {
        case 'k': case KEY_UP:    plane_shift(p, 0.0f,  SHIFT_STEP); break;
        case 'j': case KEY_DOWN:  plane_shift(p, 0.0f, -SHIFT_STEP); break;
        case 'h': case KEY_LEFT:  plane_shift(p, -SHIFT_STEP, 0.0f); break;
        case 'l': case KEY_RIGHT: plane_shift(p,  SHIFT_STEP, 0.0f); break;
        case '+': zoom_handle(p, ZOOM_IN_FACTOR);  break;
        case '-': zoom_handle(p, ZOOM_OUT_FACTOR); break;
        case 'd': p->derivative_show = !p->derivative_show; break;
        case 'r': zoom_restore(p); break;
        case 'f': expression_validate(p); break;
        case 'm': menu_options(); break;
    }
}

void
plane_init(struct Plane *p)
{
    p->xmin   = XMIN_PLANE;
    p->xmax   = XMAX_PLANE;
    p->ymin   = YMIN_PLANE;
    p->ymax   = YMAX_PLANE;
    p->xscale = XSCALE_PLANE;
    p->yscale = YSCALE_PLANE;
    p->xmaxs  = XMAX();
    p->ymaxs  = YMAX();
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
    p->xmin = PLANE_SCALE(factor, 1.0f, 0.0f, p->xmin, xctr);
    p->xmax = PLANE_SCALE(factor, 1.0f, 0.0f, p->xmax, xctr);
    p->ymin = PLANE_SCALE(factor, 1.0f, 0.0f, p->ymin, yctr);
    p->ymax = PLANE_SCALE(factor, 1.0f, 0.0f, p->ymax, yctr);  
}

void
axes_draw(const struct Plane *p)
{
    float x0 = PLANE_SCALE(0.0f, p->xmin, p->xmax, 0.0f, p->xmaxs);
    float y0 = PLANE_SCALE(0.0f, p->ymin, p->ymax, p->ymaxs, 0.0f);
    float xstep, ystep, i;
    PLANE_XSTEP(p, xstep);
    PLANE_YSTEP(p, ystep);
    for (i = 0.0f; i < p->xmaxs; i += xstep) {
        float plotx = p->xmin + xstep * i;
        int tick = fabs(fmod(plotx, p->xscale)) < xstep;
        mvaddch(y0, i, tick ? ACS_PLUS : ACS_HLINE);
    }
    for (i = 0.0f; i < p->ymaxs; i += ystep) {
        float ploty = p->ymin + ystep * i;
        int tick = fabs(fmod(ploty, p->yscale)) < ystep;
        mvaddch(i, x0, tick ? ACS_PLUS : ACS_VLINE);
    }
    mvaddch(y0, x0, ACS_PLUS);
}

void
graph_draw(const struct Plane *p)
{
    float x, xstep;
    PLANE_XSTEP(p, xstep);
    for (x = p->xmin; x <= p->xmax; x += xstep) {
        float y = p->f(x);
        attron(COLOR_PAIR(2));
        graph_plot(p, x, y);
        if (p->derivative_show) {
            float dy = evaluator_evaluate_x(p->df, x);
            attron(COLOR_PAIR(3));
            graph_plot(p, x, dy);
        }
    }
    attroff(COLOR_PAIR(3));
    attroff(COLOR_PAIR(2));
}

void
graph_plot(const struct Plane *p, float x, float y)
{
    float xp = PLANE_SCALE(x, p->xmin, p->xmax, 0.0f, p->xmaxs);
    float yp = PLANE_SCALE(y, p->ymin, p->ymax, p->ymaxs, 0.0f);
    mvaddch(yp, xp, '.');
}

void
menu_options(void)
{
    int w  = 33, h = 14;
    int wy = CENTER(YMAX(), h);
    int wx = CENTER(XMAX(), w);
    WINDOW *opts = newwin(h, w, wy, wx);
    werase(opts);
    box(opts, 0, 0);
    menu_fill(opts);
    wrefresh(opts);
    wgetch(opts);
    werase(opts);
    wrefresh(opts);
    delwin(opts);
}

void
menu_fill(WINDOW *opts)
{
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
}

int
main(int argc, char **argv)
{
#ifndef NCURSES_VERSION
    fprintf(stderr, "ncurses is needed in order to run this program.\n");
    return EXIT_FAILURE;
#endif /* NCURSES_VERSION */
    curses_init();
    struct Plane p;
    plane_init(&p);
    zoom_restore(&p);
    expression_validate(&p);
    p.derivative_show = 0;
    p.f = expression_evaluate;

    int key = 0;
    for (; key != 'q'; key = getch()) {
        keys_handle(&p, key);
        erase();
        attron(COLOR_PAIR(1));
        attron(A_REVERSE);
        attron(A_BOLD);
        mvprintw(0, 0, "f(x) = %s", evaluator_get_string(f));
        if (p.derivative_show)
            mvprintw(1, 0, "f'(x) = %s", evaluator_get_string(p.df));
        attroff(A_REVERSE);
        attroff(A_BOLD);
        axes_draw(&p);
        attroff(COLOR_PAIR(1));
        graph_draw(&p);
        refresh();
    }
    
    endwin();
    evaluator_destroy(f);   
    return EXIT_SUCCESS;
}
