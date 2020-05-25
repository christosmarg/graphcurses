#include "plane.h"

static void *f = NULL;

static void init_curses(void);
static void getfunc(Plane *p, char *buf);
static void validate_expression(Plane *p);
static float eval(float x);
static void handle_key(Plane *p, int key);

int
main(int argc, char **argv)
{
    init_curses();
    Plane p;
    plane_init(&p);
    restore_zoom(&p);
    validate_expression(&p);
    p.yfunc = eval;
    int key = 0;

    while (key != 'q')
    {
        attron(COLOR_PAIR(1));
        handle_key(&p, key);
        erase();
        attron(A_REVERSE);
        attron(A_BOLD);
        mvprintw(0, 0, "f(x) = %s", evaluator_get_string(f));
        attroff(A_REVERSE);
        attroff(A_BOLD);
        draw_axes(&p);
        attroff(COLOR_PAIR(1));

        draw_graph(&p);
        refresh();
        key = getch();
    }
    
    endwin();
    evaluator_destroy(f);   

    return 0;
}

static void
init_curses(void)
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

static void
getfunc(Plane *p, char *buf)
{
    move(0, 0);
    clrtoeol();
    printw("f(x) = ");
    echo();
    refresh();
    getnstr(buf, 256);
    restore_zoom(p);
    refresh();
    noecho();
}

static void
validate_expression(Plane *p)
{
    char *buf = (char *)malloc(256 + sizeof(char));
    getfunc(p, buf);
    while (!(f = evaluator_create(buf)))
    {
        printw("Error in expression! Try again");
        getfunc(p, buf);
        refresh();
    }
    free(buf);
}

static float
eval(float x)
{
    return evaluator_evaluate_x(f, x);
}

static void
handle_key(Plane *p, int key)
{
    switch (key)
    {
        case 'k': case KEY_UP:    shift(p, 0.0f, 1.0f); break;
        case 'j': case KEY_DOWN:  shift(p, 0.0f, -1.0f); break;
        case 'h': case KEY_LEFT:  shift(p, -1.0f, 0.0f); break;
        case 'l': case KEY_RIGHT: shift(p, 1.0f, 0.0f); break;
        case '+': handle_zoom(p, 1.0f/1.05f); break;
        case '-': handle_zoom(p, 1.05f); break;
        case 'r': restore_zoom(p); break;
        case 'f': validate_expression(p); break;
    }
}
