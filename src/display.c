#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <ncurses.h>

#include "search.h"
#include "entries.h"
#include "display.h"
#include "open.h"


#define QUIT    'q'
#define ENTER   '\n'


/* ncurse colors */
enum colors {
    normal = 1,
    yellow,
    red,
    magenta,
    green,
};

struct display {
    uint32_t index;     // position of first entry to display in entries (0->nb_entries by increment of LINES)
    int32_t cursor;     // position of cursor on screen (0->LINES)
};

extern struct search *current_search;


/* NCURSES ENVIRONMENT ********************************************************/
static void ncurses_init()
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    start_color();
    use_default_colors();
    init_pair(normal, -1, -1);
    init_pair(yellow, COLOR_YELLOW, -1);
    init_pair(red, COLOR_RED, -1);
    init_pair(magenta, COLOR_MAGENTA, -1);
    init_pair(green, COLOR_GREEN, -1);
    curs_set(0);
}

static void ncurses_stop()
{
    endwin();
}


/* PRINT STATUS ***************************************************************/
static void display_status(const struct search *search, const struct entries *entries)
{
    char *rollingwheel[4] = {"/", "-", "\\", "|"};
    static int i = 0;

    attron(COLOR_PAIR(normal));

    if (search_get_status(search)) {
        mvaddstr(0, COLS - 1, rollingwheel[++i%4]);
    } else {
        mvaddstr(0, COLS - 5, "Done.");
    }

    char nbhits[15];
    snprintf(nbhits, 15, "Hits: %d", entries_get_nb_lines(entries));
    mvaddstr(1, COLS - (int)(strchr(nbhits, '\0') - nbhits), nbhits);
}


/* PRINT DATA *****************************************************************/
static void print_line_contents(const uint32_t y_position, uint32_t line, char *data)
{
    char line_str[10] = {0};
    size_t line_str_len = snprintf(line_str, 10, "%d:", line);

    /* line number */
    attron(COLOR_PAIR(yellow));
    mvprintw(y_position, 0, "%s", line_str);

    /* line data */
    attron(COLOR_PAIR(normal));
    mvprintw(y_position, line_str_len, "%s", data);

    /* color patterns */
    char *pattern_position = NULL;
    char *pattern = search_get_pattern(current_search);

//    while ((pattern_position = strstr(data, pattern)) != NULL) { //TODO: why?
    pattern_position = strcasestr(data, pattern); {
        attron(COLOR_PAIR(red));
        mvprintw(y_position, line_str_len + pattern_position - data, "%s", pattern);
        attron(COLOR_PAIR(normal));
        data += (pattern_position - data);
    }
}

static void print_line(struct display *this,
                       const uint32_t y_position, uint32_t line, char *data)
{
    if (y_position == (uint32_t) this->cursor) {
        attron(A_REVERSE);
        print_line_contents(y_position, line, data);
        attroff(A_REVERSE);
    } else {
        print_line_contents(y_position, line, data);
    }
}

static void print_file(struct display *this, const uint32_t y_position, char *file)
{
    (void) this;

    attron(COLOR_PAIR(green));
    attron(A_BOLD);
    mvprintw(y_position, 0, "%s", file);
    attroff(A_BOLD);
}

static void display_entry(struct display *this, const struct entry *entry, const uint32_t y_position)
{
    if (entry->line == 0) {
        print_file(this, y_position, entry->data);
    } else {
        print_line(this, y_position, entry->line, entry->data);
    }
}

static void display_entries(struct display *this, const struct entries *entries)
{
    uint32_t i = 0;

    for (i = this->index; i < this->index + LINES; i++) {

        if (entries_get_data(entries, i) == NULL) {
            break;
        }

        display_entry(this, entries_get_entry(entries, i), i - this->index);
    }
}


/* MOVE COMMANDS **************************************************************/
static void page_down(struct display *this, const struct entries *entries)
{
    if (this->index + LINES > entries_get_nb_entries(entries)) {
        return;
    }

    clear();
    refresh();

    this->index += LINES;
    this->cursor = 0;

    if (entries_is_file(entries, this->index + this->cursor)) {
        this->cursor += 1;
    }

    display_entries(this, entries);
}

static void page_up(struct display *this, const struct entries *entries)
{
    if (this->index == 0) {
        this->cursor = 1;
        return;
    }

    clear();
    refresh();

    this->cursor = LINES - 1;
    this->index -= LINES;

    if (entries_is_file(entries, this->index + this->cursor)) {
        this->cursor -= 1;
    }

    display_entries(this, entries);
}

static void key_down(struct display *this, const struct entries *entries)
{
    if (entries_get_data(entries, this->index + this->cursor + 1) == NULL) {
        return;
    }

    if (this->cursor == LINES - 1) {
        page_down(this, entries);
        return;
    }

    this->cursor++;

    /* skip file */
    if (entries_is_file(entries, this->index + this->cursor)) {
        this->cursor++;
    }

    if (this->cursor > LINES - 1) {
        page_down(this, entries);
        return;
    }
}

static void key_up(struct display *this, const struct entries *entries)
{
    if (this->cursor <= 0) {
        page_up(this, entries);
        return;
    }

    this->cursor--;

    /* skip file */
    if (entries_is_file(entries, this->index + this->cursor)) {
        this->cursor--;
    }

    if (this->cursor < 0) {
        page_up(this, entries);
        return;
    }
}


/* API ************************************************************************/
void display_loop(struct display *this, const struct search *search, const struct entries *entries)
{
    uint8_t run = 1;
    int ch = 0;

    ncurses_init();

    while ((ch = getch()) && run) {
        switch(ch) {

        case KEY_NPAGE:
            page_down(this, entries);
            break;

        case KEY_PPAGE:
            page_up(this, entries);
            break;

        case KEY_DOWN:
            key_down(this, entries);
            break;

        case KEY_UP:
            key_up(this, entries);
            break;

        case QUIT:
            run = 0;
            break;

        case ENTER:
            ncurses_stop();
            open_entry(entries, this->index + this->cursor);
            ncurses_init();
            break;

        default:
            break;
        }

        usleep(10000);
        refresh();
        display_entries(this, entries);
        display_status(search, entries);

        /* check if search thread has ended without results */
        if (!search_get_status(search) && entries->nb_entries == 0) {
            run = 0;
        }
    }

    ncurses_stop();
}


/* CONSTRUCTOR ****************************************************************/
struct display * display_new(void)
{
    struct display *this = calloc(1, sizeof(struct display));

    return this;
}

void display_delete(struct display *this)
{
    free(this);
}
