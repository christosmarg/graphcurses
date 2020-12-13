/* See LICENSE file for copyright and license details. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <matheval.h>
#include <ncurses.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif /* M_PI */

#define XMIN_PLANE      (-2.0f * M_PI)
#define XMAX_PLANE      ( 2.0f * M_PI)
#define YMIN_PLANE      (-M_PI)
#define YMAX_PLANE       M_PI
#define XSCALE_PLANE     1.0f
#define YSCALE_PLANE     1.0f
#define SHIFT_STEP       1.0f
#define ZOOM_IN_FACTOR  (1.0f / 1.05f)
#define ZOOM_OUT_FACTOR  1.05f
#define BUFSIZE          256

#define YMAX            (getmaxy(stdscr))
#define XMAX            (getmaxx(stdscr))
#define CENTER(x, y)    ((x) / 2 - (y) / 2)
#define PLANE_SCALE(val, omin, omax, nmin, nmax)                              \
        ((((val) - (omin)) / ((omax) - (omin))) * ((nmax) - (nmin)) + (nmin))
#define PLANE_XSTEP(p, xstep)                                                 \
        xstep = (p->xmax - p->xmin) / (p->xmaxs + 1.0f);                       
#define PLANE_YSTEP(p, ystep)                                                 \
        ystep = (p->xmax - p->ymin) / (p->ymaxs + 1.0f);

#define OPT_QUIT            "q              Quit"
#define OPT_MOVE_UP         "Up/k           Move up"
#define OPT_MOVE_DOWN       "Down/j         Move down"
#define OPT_MOVE_LEFT       "Left/h         Move left"
#define OPT_MOVE_RIGHT      "Right/l        Move right"
#define OPT_SHOW_DERIVATIVE "d              Show derivative"
#define OPT_NEW_FUNCTION    "f              New function"
#define OPT_RESTORE_ZOOM    "r              Restore zoom"
#define OPT_ZOOM_IN         "+              Zoom in"
#define OPT_ZOOM_OUT        "-              Zoom out"
#define MSG_QUIT_MENU       "Press any key to quit the menu"

typedef struct {
        float   (*f)(float);
        void    *df;
        float    ymin;
        float    ymax;
        float    xmin;
        float    xmax;
        float    xscale;
        float    yscale;
        int      ymaxs;
        int      xmaxs;
        int      derivshow;
} Plane;

static void     cursesinit(void);
static void     funcget(Plane *, char *);
static void     exprvalidate(Plane *);
static float    expreval(float);
static Plane   *planeinit(void);
static void     planeshift(Plane *, float, float);
static void     zoomrestore(Plane *);
static void     zoomhandle(Plane *, float);
static void     axesdraw(const Plane *);
static void     graphdraw(const Plane *);
static void     graphplot(const Plane *, float, float);
static void     menuopts(void);

static void *f  = NULL;

static void
cursesinit(void)
{
        if (!initscr()) {
                fputs("graphcurses: initscr: cannot initialiaze ncurses", stderr);
                exit(EXIT_FAILURE);
        }
        cbreak();
        noecho();
        curs_set(0);
        keypad(stdscr, 1);

        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_CYAN, COLOR_BLACK);
        init_pair(3, COLOR_RED, COLOR_BLACK);
}

static void
funcget(Plane *p, char *buf)
{
        move(0, 0);
        clrtoeol();
        printw("f(x) = ");
        echo();
        refresh();
        getnstr(buf, BUFSIZE);
        zoomrestore(p);
        refresh();
        noecho();
}

static void
exprvalidate(Plane *p)
{
        char *buf;

        if ((buf = malloc(BUFSIZE)) == NULL) {
                fputs("graphcurses: exprvalidate: cannot allocate memory", stderr);
                exit(EXIT_FAILURE);
        }
        funcget(p, buf);
        while (!(f = evaluator_create(buf))) {
                printw("Error in expression! Try again");
                funcget(p, buf);
                refresh();
        }
        p->df = evaluator_derivative_x(f);
        free(buf);
}

static float
expreval(float x)
{
        return evaluator_evaluate_x(f, x);
}

static Plane *
planeinit(void)
{
        Plane *p;

        if ((p = malloc(sizeof(Plane))) == NULL) {
                fputs("graphcurses: planeinit: cannot allocate memory", stderr);
                exit(EXIT_FAILURE);
        }

        p->xmaxs = XMAX;
        p->ymaxs = YMAX;
        p->derivshow = 0;
        p->f = expreval;
        zoomrestore(p);

        return p;
}

static void
planeshift(Plane *p, float xshift, float yshift)
{
        xshift *= (p->xmax - p->xmin) / 16.0f;
        yshift *= (p->ymax - p->ymin) / 16.0f;
        p->xmin += xshift;
        p->xmax += xshift;
        p->ymin += yshift;
        p->ymax += yshift;
}

static void
zoomrestore(Plane *p)
{
        p->xmin = XMIN_PLANE;
        p->xmax = XMAX_PLANE;
        p->ymin = YMIN_PLANE;
        p->ymax = YMAX_PLANE;
        p->xscale = XSCALE_PLANE;
        p->yscale = YSCALE_PLANE;
}

static void
zoomhandle(Plane *p, float factor)
{
        float xctr = (p->xmin + p->ymax) / 2.0f;
        float yctr = (p->ymin + p->ymax) / 2.0f;

        p->xmin = PLANE_SCALE(factor, 1.0f, 0.0f, p->xmin, xctr);
        p->xmax = PLANE_SCALE(factor, 1.0f, 0.0f, p->xmax, xctr);
        p->ymin = PLANE_SCALE(factor, 1.0f, 0.0f, p->ymin, yctr);
        p->ymax = PLANE_SCALE(factor, 1.0f, 0.0f, p->ymax, yctr);  
}

static void
axesdraw(const Plane *p)
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
graphdraw(const Plane *p)
{
        float x, y, dy, xstep;

        PLANE_XSTEP(p, xstep);
        for (x = p->xmin; x <= p->xmax; x += xstep) {
                y = p->f(x);
                attron(COLOR_PAIR(2));
                graphplot(p, x, y);
                if (p->derivshow) {
                        dy = evaluator_evaluate_x(p->df, x);
                        attron(COLOR_PAIR(3));
                        graphplot(p, x, dy);
                }
        }
        attroff(COLOR_PAIR(2) | COLOR_PAIR(3));
}

static void
graphplot(const Plane *p, float x, float y)
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
        opts = newwin(h, w, wy, wx);
        werase(opts);
        box(opts, 0, 0);

        /* fill menu */
        mvwprintw(opts, 1,  1, OPT_QUIT);
        mvwprintw(opts, 2,  1, OPT_MOVE_UP);
        mvwprintw(opts, 3,  1, OPT_MOVE_DOWN);
        mvwprintw(opts, 4,  1, OPT_MOVE_LEFT);
        mvwprintw(opts, 5,  1, OPT_MOVE_RIGHT);
        mvwprintw(opts, 6,  1, OPT_SHOW_DERIVATIVE);
        mvwprintw(opts, 7,  1, OPT_NEW_FUNCTION);
        mvwprintw(opts, 8,  1, OPT_RESTORE_ZOOM);
        mvwprintw(opts, 9,  1, OPT_ZOOM_IN);
        mvwprintw(opts, 10, 1, OPT_ZOOM_OUT);
        mvwprintw(opts, 12, 1, MSG_QUIT_MENU);

        wrefresh(opts);
        wgetch(opts);
        werase(opts);
        wrefresh(opts);
        delwin(opts);
}

int
main(int argc, char *argv[])
{
        /* TODO: implement `die` */
        Plane *p;
        int key = 0;

        cursesinit();
        p = planeinit();
        exprvalidate(p);

        for (; key != 'q'; key = getch()) {
                switch (key) {
                case 'k':       /* FALLTHROUGH */
                case KEY_UP:
                        planeshift(p, 0.0f,  SHIFT_STEP);
                        break;
                case 'j':       /* FALLTHROUGH */
                case KEY_DOWN:
                        planeshift(p, 0.0f, -SHIFT_STEP);
                        break;
                case 'h':       /* FALLTHROUGH */
                case KEY_LEFT:
                        planeshift(p, -SHIFT_STEP, 0.0f);
                        break;
                case 'l':       /* FALLTHROUGH */
                case KEY_RIGHT:
                        planeshift(p,  SHIFT_STEP, 0.0f);
                        break;
                case '+':
                        zoomhandle(p, ZOOM_IN_FACTOR);
                        break;
                case '-':
                        zoomhandle(p, ZOOM_OUT_FACTOR);
                        break;
                case 'd':
                        p->derivshow ^= 1;
                        break;
                case 'r':
                        zoomrestore(p);
                        break;
                case 'f':
                        exprvalidate(p);
                        break;
                case 'm':
                        menuopts();
                        break;
                }

                erase();
                attron(COLOR_PAIR(1) | A_REVERSE | A_BOLD);
                mvprintw(0, 0, "f(x) = %s", evaluator_get_string(f));
                if (p->derivshow)
                        mvprintw(1, 0, "f'(x) = %s", evaluator_get_string(p->df));
                attroff(COLOR_PAIR(1) | A_REVERSE | A_BOLD);

                axesdraw(p);
                graphdraw(p);
                refresh();
        }
        
        endwin();
        free(p);
        evaluator_destroy(f);   

        return EXIT_SUCCESS;
}
