#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <ncurses.h>

#include <signal.h>

#include "display.h"
#include "entries.h"
#include "open.h"
#include "search.h"
#include "subsearch.h"


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

    struct display *parent;
};

extern struct search *current_search;


/* NCURSES ENVIRONMENT ********************************************************/
static void ncurses_init(void)
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

static void ncurses_stop(void)
{
    endwin();
}

static void ncurses_clear_screen(void)
{
    clear();
    refresh();
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
static void print_line_contents(const uint32_t y_position,
                                const uint32_t line_number,
                                char *line_contents)
{
    char line_str[10] = {0};
    size_t line_str_len = snprintf(line_str, 10, "%d:", line_number);

    /* print the line number */
    attron(COLOR_PAIR(yellow));
    mvprintw(y_position, 0, "%s", line_str);

    /* first, print whole line contents */
    attron(COLOR_PAIR(normal));
    mvprintw(y_position, line_str_len, "%.*s", COLS - line_str_len, line_contents);

    char *pattern = search_get_pattern(current_search);

    /* next, color all patterns on line */
    char *pattern_position = NULL;
    char *ptr = line_contents;
    move(y_position, line_str_len); // reset cursor to beginning of line

    /* find next occurrence of pattern */
    while ((pattern_position = strcasestr(ptr, pattern))) {

        /* return if pattern is off-screen */
        if (pattern_position - line_contents > COLS) {
            break;
        }

        /* move by 1 char until pattern is reached */
        while (ptr < pattern_position) {
            addch(*ptr);
            ptr++;
        }

        /* print pattern then move ptr by pattern size */
        attron(COLOR_PAIR(red));
        printw("%s", pattern);
        attron(COLOR_PAIR(normal));
        ptr += strlen(pattern);
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
    mvprintw(y_position, 0, "%.*s", COLS, file);
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
    uint32_t nb_entries = entries_get_nb_entries(entries);

    /* if there isn't a next page, move to the last entry on this page */
    if (this->index + LINES >= nb_entries) {
        this->cursor = nb_entries - this->index - 1;
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


/* GOTO LINE NUMBER ***********************************************************/
static void goto_home(struct display *this)
{
    this->index = 0;
    this->cursor = 1;
}

static void goto_end(struct display *this, const struct entries *entries)
{
    uint32_t nb_entries = entries_get_nb_entries(entries);

    this->index = (nb_entries / LINES) * LINES;
    this->cursor = nb_entries % LINES - 1;
}


/* SUBSEARCH ******************************************************************/
/**
 * Pops a new window for the user to write a new pattern to look for.
 * Warning: caller is responsible for freeing returned pattern.
 *
 * @return  New pattern
 */
static char * subsearch_window(const uint8_t invert)
{
	WINDOW	*searchw;
	int	j = 0, car;

    char *search = calloc(4096, sizeof(char));

	searchw = newwin(3, 50, (LINES-3)/2 , (COLS-50)/2);
	box(searchw, 0,0);
	wrefresh(searchw);
	refresh();

    char *include = "To include: ";
    char *exclude = "To exclude: ";
    char *include_format = "To include: %s ";
    char *exclude_format = "To exclude: %s ";
    char *text = NULL;
    char *format = NULL;

    if (invert) {
        text = exclude;
        format = exclude_format;
    } else {
        text = include;
        format = include_format;
    }
    mvwprintw(searchw, 1, 1, text, NULL);

	while ((car = wgetch(searchw)) != '\n' && j < 4096) {
		if (car == 8 || car == 127) { //backspace
			if (j > 0)
				search[--j] = 0;
			mvwprintw(searchw, 1, 1, format, search);
			continue;
		}

		if (car == 27) { //escape
            free(search);
            delwin(searchw);
            return NULL;
		}

		search[j++] = car;
		mvwprintw(searchw, 1, 1, format, search);
	}
	search[j] = 0;
	delwin(searchw);
    ncurses_clear_screen();

    if (j <= 0) {
        free(search);
        return NULL;
    }

    return search;
}


/* API ************************************************************************/
void display_loop(struct display *this, const struct search *search)
{
    uint8_t run = 1;
    int ch = 0;
    struct entries *entries = search_get_entries(search);

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

        case KEY_RESIZE:
            ncurses_clear_screen();
            break;

        case QUIT: {
            struct search *parent_search = search_get_parent(current_search);
            if (parent_search) {
                subsearch_delete(current_search);
                current_search = parent_search;
                entries = search_get_entries(current_search);
                this = this->parent;
                ncurses_clear_screen();
            } else {
                run = 0;
            }
            break;
        }

        case ENTER:
            ncurses_stop();
            open_entry(entries, this->index + this->cursor);
            ncurses_init();
            break;

        /* subsearch include */
        case '/': {
            char *sub_pattern = subsearch_window(0);
            if (sub_pattern == NULL) {
                ncurses_clear_screen();
                break;
            }
            struct search *subsearch = subsearch_new(current_search, sub_pattern, 0);
            free(sub_pattern);
            subsearch_search(subsearch);
            current_search = subsearch;
            entries = search_get_entries(current_search);

            struct display *subdisplay = display_new();
            subdisplay->parent = this;
            this = subdisplay;

            ncurses_clear_screen();
            break;
        }

        /* subsearch exclude */
        case '\\': {    //TODO: clean this up
            char *sub_pattern = subsearch_window(1);
            if (sub_pattern == NULL) {
                ncurses_clear_screen();
                break;
            }
            struct search *subsearch = subsearch_new(current_search, sub_pattern, 1);
            free(sub_pattern);
            subsearch_search(subsearch);
            current_search = subsearch;
            entries = search_get_entries(current_search);

            struct display *subdisplay = display_new();
            subdisplay->parent = this;
            this = subdisplay;

            ncurses_clear_screen();
            break;
        }

        /* goto line number */
        case KEY_HOME:
            goto_home(this);
            ncurses_clear_screen();
            break;

        case KEY_END:
            goto_end(this, entries);
            ncurses_clear_screen();
            break;

        default:
            break;
        }

        usleep(10000);
        refresh();
        display_entries(this, entries);
        display_status(search, entries);

        /* check if search thread has ended without results */
        if (!search_get_status(search) && entries->nb_entries == 0 && !search_get_parent(current_search)) {
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
