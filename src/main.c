#include "plane.h"

#define BUFFSIZE 256

static void  *f = NULL;

static void  curses_init(void);
static void  func_get(Plane *p, char *buf);
static void  expression_validate(Plane *p);
static float expression_evaluate(float x);
static void  keys_handle(Plane *p, int key);

int
main(int argc, char **argv)
{
#ifndef NCURSES_VERSION
    fprintf(stderr, "ncurses is needed in order to run this program.\n");
    return EXIT_FAILURE;
#endif /* NCURSES_VERSION */
    curses_init();
    Plane p;
    plane_init(&p);
    zoom_restore(&p);
    expression_validate(&p);
    p.yfunc = expression_evaluate;

    int key = 0;
    while (key != 'q')
    {
        attron(COLOR_PAIR(1));
        keys_handle(&p, key);
        erase();
        attron(A_REVERSE);
        attron(A_BOLD);
        mvprintw(0, 0, "f(x) = %s", evaluator_get_string(f));
        attroff(A_REVERSE);
        attroff(A_BOLD);
        axes_draw(&p);
        attroff(COLOR_PAIR(1));
        graph_draw(&p);
        refresh();
        key = getch();
    }
    
    endwin();
    evaluator_destroy(f);   
    return EXIT_SUCCESS;
}

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
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
}

void
func_get(Plane *p, char *buf)
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
expression_validate(Plane *p)
{
    char *buf = (char *)malloc(BUFFSIZE + sizeof(char));
    func_get(p, buf);
    while (!(f = evaluator_create(buf)))
    {
        printw("Error in expression! Try again");
        func_get(p, buf);
        refresh();
    }
    free(buf);
}

float
expression_evaluate(float x)
{
    return evaluator_evaluate_x(f, x);
}

void
keys_handle(Plane *p, int key)
{
    switch (key)
    {
        case 'k': case KEY_UP:    plane_shift(p, 0.0f,  SHIFT_STEP); break;
        case 'j': case KEY_DOWN:  plane_shift(p, 0.0f, -SHIFT_STEP); break;
        case 'h': case KEY_LEFT:  plane_shift(p, -SHIFT_STEP, 0.0f); break;
        case 'l': case KEY_RIGHT: plane_shift(p,  SHIFT_STEP, 0.0f); break;
        case '+': zoom_handle(p, ZOOM_IN_FACTOR);  break;
        case '-': zoom_handle(p, ZOOM_OUT_FACTOR); break;
        case 'r': zoom_restore(p); break;
        case 'f': expression_validate(p); break;
    }
}
