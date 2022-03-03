#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <locale.h>
#include <ncurses.h>

#include <signal.h>

#include "display.h"
#include "entries.h"
#include "open.h"
#include "search.h"
#include "subsearch.h"


#define QUIT        'q'
#define ENTER       '\n'
#define ESCAPE      27
#define BACKSPACE   8
#define SUPPR       127


/* ncurse colors */
enum colors {
    normal = 1,
    yellow,
    red,
    magenta,
    green,
};

struct display {
    /* positions of current session */
    uint32_t index;     // position of first entry to display in entries (0->nb_entries by increment of (LINES - 1))
    int32_t cursor;     // position of cursor on screen (0->(LINES - 1))

    struct display *parent_display;
    char *patterns;     // patterns for the status bar

    int display_vertical_size;
};

extern struct search *current_search;


/* NCURSES ENVIRONMENT ********************************************************/
static void ncurses_init(void)
{
    setlocale(LC_ALL, "");
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


/* PRINT SEARCH TO FILE *******************************************************/
static void save_search_output(const struct entries *entries)
{
    uint32_t nb_entries = entries_get_nb_entries(entries);

    FILE *output = fopen("ngp.out", "w+");
    if (!output) {
        return;
    }

    uint32_t i = 0;
    for (i = 0; i < nb_entries; i++) {
        char *data = entries_get_data(entries, i);
        fprintf(output, "%s\n", data);
    }

    fclose(output);
}


/* PRINT STATUS ***************************************************************/
static void display_bar(struct display *this, const struct search *search, const struct entries *entries)
{
    static int i = 0;
    char *rollingwheel[4] = {"/", "-", "\\", "|"};

    /* decide rolling wheel character */
    char *roll_char = ".";
    if (search_get_status(search)) {
        roll_char = rollingwheel[++i%4];
    }

    /* build first part of status line that shows patterns */
    char *buf = calloc(COLS, sizeof(char));
    snprintf(buf, COLS, "%s%*s", this->patterns, (int)(COLS - strlen(this->patterns)), "");

    /* calculate percent of entries scrolled */
    int percent_completed = 0;
    uint32_t nb_entries = entries_get_nb_entries(entries);
    if (nb_entries) {
        percent_completed = (100 * (this->index + this->cursor + 1)) / nb_entries;
    }

    /* build second part of status line that shows number of entries */
    char tmp[256] = {0};
    snprintf(tmp, 256, "   %d %d%% %s", entries_get_nb_lines(entries), percent_completed, roll_char);

    /* fuuuuuuu-sion */
    memcpy(buf + COLS - strlen(tmp) - 1, tmp, strlen(tmp));
    buf[COLS - 1] = 0;

    /* print status line */
    attron(COLOR_PAIR(normal));
    mvaddstr(LINES - 1, 0, buf);
    free(buf);
}


/* PRINT DATA *****************************************************************/
/**
 * Find and colorize regex patterns.
 */
static void colorize_regex_pattern(char *line_contents)
{
    /* only one match cuz I'm lazy */
    regmatch_t pmatch[1];
    regexec(search_get_regex(current_search), line_contents, 1, pmatch, 0);
    size_t start = pmatch[0].rm_so;
    int stop = pmatch[0].rm_eo;

    char *ptr = line_contents;

    if ((int)start > COLS) {
        return;
    }

    printw("%.*s", (int)start, ptr);
    ptr += start;

    attron(COLOR_PAIR(red));

    int max_print = 0;
    if (stop > COLS) {
        max_print = COLS - start;
    } else {
        max_print = stop - start;
    }

    printw("%.*s", max_print, ptr);
    ptr += start;

    attron(COLOR_PAIR(normal));
}

/**
 * Find and colorize normal patterns.
 */
static void colorize_normal_patterns(char *line_contents, const uint8_t visited)
{
    char *ptr = line_contents;

    char *pattern = search_get_pattern(current_search);
    char *pattern_position = NULL;

    char *(*search_function)(const char *, const char *);

    if (search_get_sensitive(current_search)) {
        search_function = strcasestr;
    } else {
        search_function = strstr;
    }

    /* find next occurrence of pattern */
    while ((pattern_position = search_function(ptr, pattern))) {

        /* return if pattern is off-screen */
        if (pattern_position - line_contents > COLS) {
            break;
        }

        printw("%.*s", (int)(pattern_position - ptr), ptr);
        ptr += pattern_position - ptr;

        /* print pattern then move ptr by pattern size */
        attron(COLOR_PAIR(red));
        printw("%.*s", (int)strlen(pattern), pattern_position);
        if (visited) {
            attron(COLOR_PAIR(magenta));
        } else {
            attron(COLOR_PAIR(normal));
        }
        ptr += strlen(pattern);
    }
}

static void print_line_contents(const uint32_t y_position,
                                const uint32_t line_number,
                                char *line_contents,
                                const uint8_t visited)
{
    char line_str[10] = {0};
    size_t line_str_len = snprintf(line_str, 10, "%d:", line_number);

    /* print the line number */
    attron(COLOR_PAIR(yellow));
    mvprintw(y_position, 0, "%s", line_str);

    /* first, print whole line contents */
    if (visited) {
        attron(A_REVERSE);
        attron(COLOR_PAIR(magenta));
    } else {
        attron(COLOR_PAIR(normal));
    }
    mvprintw(y_position, line_str_len, "%.*s", (int)(COLS - line_str_len), line_contents);
    move(y_position, line_str_len); // reset cursor to beginning of line

    /* next, overwrite patterns on the line */
    if (search_get_regex(current_search)) {
        colorize_regex_pattern(line_contents);
    } else {
        colorize_normal_patterns(line_contents, visited);
    }

    /* reset colors if need be */
    if (visited) {
        attron(COLOR_PAIR(normal));
        attroff(A_REVERSE);
    }
}

static void print_line(struct display *this,
                       const uint32_t y_position, uint32_t line, char *data,
                       const uint8_t visited)
{
    if (y_position == (uint32_t) this->cursor) {
        attron(A_REVERSE);
        print_line_contents(y_position, line, data, 0);
        attroff(A_REVERSE);
    } else {
        print_line_contents(y_position, line, data, visited);
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
        print_line(this, y_position, entry->line, entry->data, entry->visited);
    }
}

static void display_entries(struct display *this, const struct entries *entries)
{
    uint32_t i = 0;

    for (i = this->index; i < this->index + (LINES - 1); i++) {

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
    if (this->index + (LINES - 1) >= nb_entries) {
        this->cursor = nb_entries - this->index - 1;
        return;
    }

    this->index += (LINES - 1);
    this->cursor = 0;

    if (entries_is_file(entries, this->index + this->cursor)) {
        this->cursor += 1;
    }

    clear();
    display_entries(this, entries);
}

static void page_up(struct display *this, const struct entries *entries)
{
    if (this->index == 0) {
        this->cursor = 1;
        return;
    }

    this->cursor = (LINES - 1) - 1;
    this->index -= (LINES - 1);

    if (entries_is_file(entries, this->index + this->cursor)) {
        this->cursor -= 1;
    }

    clear();
    display_entries(this, entries);
}

static void key_down(struct display *this, const struct entries *entries)
{
    if (entries_get_data(entries, this->index + this->cursor + 1) == NULL) {
        return;
    }

    if (this->cursor == (LINES - 1) - 1) {
        page_down(this, entries);
        return;
    }

    this->cursor++;

    /* skip file */
    if (entries_is_file(entries, this->index + this->cursor)) {
        this->cursor++;
    }

    if (this->cursor > (LINES - 1) - 1) {
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
    clear();
}

static void goto_end(struct display *this, const struct entries *entries)
{
    uint32_t nb_entries = entries_get_nb_entries(entries);

    this->index = (nb_entries / (LINES - 1)) * (LINES - 1);
    this->cursor = nb_entries % (LINES - 1) - 1;
    clear();
}


/* SUBSEARCH ******************************************************************/
static void print_mode_window(WINDOW *modew,
                              const struct subsearch_user_params *user_param)
{
    /* regular string */
    if (user_param->search_type == search_type_string) {
        wattron(modew, A_REVERSE);
    }
    mvwprintw(modew, 1, 1, "%s", "string");
    wattroff(modew, A_REVERSE);

    /* nocase string */
    if (user_param->search_type == search_type_nocase) {
        wattron(modew, A_REVERSE);
    }
    mvwprintw(modew, 2, 1, "%s", "nocase");
    wattroff(modew, A_REVERSE);

    /* regex */
    if (user_param->search_type == search_type_regex) {
        wattron(modew, A_REVERSE);
    }
    mvwprintw(modew, 3, 1, "%s", "regex");
    wattroff(modew, A_REVERSE);

    wrefresh(modew);
}

/**
 * Pops a new window for the user to write a new pattern to look for.
 */
static uint8_t subsearch_window(struct subsearch_user_params *user_param)
{
	int	j = 0, car;

    char *search = user_param->pattern;

    WINDOW *modew = newwin(5, 8, ((LINES - 1)-5)/2 , (COLS-50)/2 - 7);
    box(modew, 0, 0);
    print_mode_window(modew, user_param);

	WINDOW *searchw = newwin(3, 50, ((LINES - 1)-3)/2 , (COLS-50)/2);
	box(searchw, 0, 0);

    char *include_format = "To include: %s ";
    char *exclude_format = "To exclude: %s ";
    char *format = NULL;

    if (user_param->invert_search) {
        format = exclude_format;
    } else {
        format = include_format;
    }
    mvwprintw(searchw, 1, 1, format, "");

	while ((car = wgetch(searchw)) != '\n' && j < 4095) {

		if (car == ESCAPE) {

            nodelay(searchw, TRUE);
            car = wgetch(searchw);

            /* no char received after means ESCAPE key */
            if (car == ERR) {
                delwin(searchw);
	            delwin(modew);
                return EXIT_FAILURE;
            }

            /* extended keycodes */
            if (car == 91) {
                car = wgetch(searchw);

                /* up key */
                if (car == 65) {
                    if (user_param->search_type > 0) {
                        user_param->search_type--;
                    }
                }

                /* down key */
                if (car == 66) {
                    if (user_param->search_type < 2) {
                        user_param->search_type++;
                    }
                }

                print_mode_window(modew, user_param);
            }

            nodelay(searchw, FALSE);
            continue;
		}

		if (car == BACKSPACE || car == SUPPR) {
			if (j > 0) {
				search[--j] = 0;
            }
			mvwprintw(searchw, 1, 1, format, search);
			continue;
		}

		search[j++] = car;
		mvwprintw(searchw, 1, 1, format, search);
	}

	search[j] = 0;
	delwin(searchw);
	delwin(modew);

    if (j <= 0) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


/* API ************************************************************************/
static void resize(struct display *this)
{
    /* realign indexes with new vertical size so that the first entry
       is always at the same place in the display */
    uint32_t current_position = this->index + this->cursor;

    this->display_vertical_size = LINES;
    this->cursor = current_position % (this->display_vertical_size - 1);
    this->index = current_position - this->cursor;
    ncurses_clear_screen();
}

void display_loop(struct display *this, const struct search *main_search)
{
    uint8_t run = 1;
    int ch = 0;
    struct entries *entries = search_get_entries(main_search);

    ncurses_init();
    while ((ch = getch()) && run) {
        int sleep_time = 10000;
        switch(ch) {

        case KEY_NPAGE:
            page_down(this, entries);
            sleep_time = 0;
            break;

        case KEY_PPAGE:
            page_up(this, entries);
            sleep_time = 0;
            break;

        case KEY_DOWN:
            key_down(this, entries);
            sleep_time = 0;
            break;

        case KEY_UP:
            key_up(this, entries);
            sleep_time = 0;
            break;

        /* goto line number */
        case KEY_HOME:
            goto_home(this);
            ncurses_clear_screen();
            sleep_time = 0;
            break;

        case KEY_END:
            goto_end(this, entries);
            ncurses_clear_screen();
            sleep_time = 0;
            break;

        case KEY_RESIZE: {
            resize(this);
            break;
        }

        case ESCAPE:
        case QUIT: {
            struct search *parent_search = search_get_parent(current_search);
            if (parent_search) {
                subsearch_delete(current_search);
                current_search = parent_search;
                entries = search_get_entries(current_search);
                struct display *subdisplay = this;
                this = this->parent_display;
                display_delete(subdisplay);

                /* force parent resize because child might have resized too
                   and since we're reusing old indexes they might need to be
                   realigned */
                resize(this);
            } else {
                run = 0;
            }
            break;
        }

        case ENTER: {
            uint32_t entry_index = this->index + this->cursor;
            if (entry_index == 0) {
                break;
            }

            ncurses_stop();
            open_entry(entries, entry_index);
            ncurses_init();
            break;
        }

        /* subsearch, include user pattern */
        case '/': {
            struct subsearch_user_params user_params = {0};

            uint8_t result = subsearch_window(&user_params);
            if (result == EXIT_FAILURE) {
                ncurses_clear_screen();
                break;
            }

            struct search *subsearch = subsearch_new(current_search, &user_params);
            current_search = subsearch;
            entries = search_get_entries(current_search);

            struct display *subdisplay = display_new(this, user_params.pattern);
            this = subdisplay;

            ncurses_clear_screen();
            break;
        }

        /* subsearch, exclude user pattern */
        case '\\': {    //TODO: clean this up
            struct subsearch_user_params user_params = {0};
            user_params.invert_search = 1;

            uint8_t result = subsearch_window(&user_params);
            if (result == EXIT_FAILURE) {
                ncurses_clear_screen();
                break;
            }

            struct search *subsearch = subsearch_new(current_search, &user_params);
            current_search = subsearch;
            entries = search_get_entries(current_search);

            struct display *subdisplay = display_new(this, user_params.pattern);
            this = subdisplay;

            ncurses_clear_screen();
            break;
        }

        case ' ':
            entries_toggle_visited(entries, this->index + this->cursor);
            break;

        case 'p':
            save_search_output(entries);
            break;

        default:
            break;
        }

        usleep(sleep_time);
        display_entries(this, entries);
        display_bar(this, main_search, entries);

        /* check if main search thread has ended without results */
        if (!search_get_parent(current_search) &&
            !search_get_status(main_search) && entries->nb_entries == 0) {
            run = 0;
        }
    }

    ncurses_stop();
}


/* CONSTRUCTOR ****************************************************************/
/**
 * Create an independant display.
 * Argument pattern is used to build the bottom status bar showing all patterns.
 */
struct display * display_new(struct display *parent_display, char *pattern)
{
    struct display *this = calloc(1, sizeof(struct display));

    this->parent_display = parent_display;
    if (this->parent_display) {
        size_t length = strlen(this->parent_display->patterns) + 2 + strlen(pattern) + 1;
        this->patterns = calloc(length, sizeof(char));

        char search_invert;
        if (search_get_invert(current_search)) {
            search_invert = '\\';
        } else {
            search_invert = '/';
        }

        snprintf(this->patterns, length, "%s %c%s", this->parent_display->patterns, search_invert, pattern);
    } else {
        this->patterns = strdup(pattern);
    }

    this->display_vertical_size = LINES;

    return this;
}

void display_delete(struct display *this)
{
    free(this->patterns);
    free(this);
}
