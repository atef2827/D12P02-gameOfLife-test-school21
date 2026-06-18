/* The Game of Life -- Conway's cellular automaton.
 *
 * The universe is a self-contained (toroidal) board of ROWS x COLS cells.
 * The initial state is read from stdin (so it can be redirected from a file),
 * while interactive controls are read from the controlling terminal /dev/tty.
 *
 * Controls:
 *   A / a  -> speed up   (smaller delay between generations)
 *   Z / z  -> slow down  (larger delay between generations)
 *   Space  -> end the game
 *
 * The code follows E. Dijkstra's structured programming principles:
 * no goto, no global variables, a single entry and exit per construct,
 * nesting depth kept shallow, and small single-purpose functions.
 */

#include <ncurses.h>
#include <stdio.h>

#define ROWS 25
#define COLS 80

#define CELL_ALIVE 'O'
#define CELL_DEAD ' '

#define DELAY_START 200
#define DELAY_STEP 25
#define DELAY_MIN 25
#define DELAY_MAX 1000

#define KEY_FASTER_LOWER 'a'
#define KEY_FASTER_UPPER 'A'
#define KEY_SLOWER_LOWER 'z'
#define KEY_SLOWER_UPPER 'Z'
#define KEY_QUIT ' '

typedef struct {
    int cells[ROWS][COLS];
} Field;

int main(void);
static int char_is_alive(int ch);
static void read_field(Field* field);
static int count_neighbours(const Field* field, int row, int col);
static void next_generation(const Field* current, Field* next);
static int count_population(const Field* field);
static void draw_field(const Field* field);
static void draw_status(int generation, int population, int delay);
static int adjust_delay(int delay, int ch);
static SCREEN* init_screen(FILE* terminal);
static void run_game(Field* current, Field* next);

int main(void) {
    Field current = {0};
    Field next = {0};
    int status = 0;
    read_field(&current);
    FILE* terminal = fopen("/dev/tty", "r");
    SCREEN* screen = (terminal != NULL) ? init_screen(terminal) : NULL;
    if (screen == NULL) {
        fprintf(stderr, "Error: an interactive terminal is required to play.\n");
        status = 1;
    } else {
        run_game(&current, &next);
        endwin();
        delscreen(screen);
    }
    if (terminal != NULL) {
        fclose(terminal);
    }
    return status;
}

/* Any cell marked with a "filled" glyph is treated as alive; everything
 * else (dots, zeros, spaces, ...) is treated as dead. */
static int char_is_alive(int ch) {
    return ch == '1' || ch == '*' || ch == 'O' || ch == 'o' || ch == '#' || ch == 'X' || ch == 'x' ||
           ch == '@';
}

/* Read the seed matrix from stdin, one terminal row per text line. */
static void read_field(Field* field) {
    int row = 0;
    int col = 0;
    int ch = 0;
    while (row < ROWS && (ch = fgetc(stdin)) != EOF) {
        if (ch == '\n') {
            row++;
            col = 0;
        } else if (col < COLS) {
            field->cells[row][col] = char_is_alive(ch);
            col++;
        }
    }
}

/* Count the live neighbours of a cell, wrapping around the edges so the
 * board behaves like a torus. */
static int count_neighbours(const Field* field, int row, int col) {
    int count = 0;
    for (int d_row = -1; d_row <= 1; d_row++) {
        for (int d_col = -1; d_col <= 1; d_col++) {
            if (d_row != 0 || d_col != 0) {
                int neighbour_row = (row + d_row + ROWS) % ROWS;
                int neighbour_col = (col + d_col + COLS) % COLS;
                count += field->cells[neighbour_row][neighbour_col];
            }
        }
    }
    return count;
}

/* Apply Conway's rules simultaneously to produce the next generation. */
static void next_generation(const Field* current, Field* next) {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            int neighbours = count_neighbours(current, row, col);
            int alive = current->cells[row][col];
            int survives = (neighbours == 3) || (alive && neighbours == 2);
            next->cells[row][col] = survives ? 1 : 0;
        }
    }
}

static int count_population(const Field* field) {
    int population = 0;
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            population += field->cells[row][col];
        }
    }
    return population;
}

static void draw_field(const Field* field) {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            int glyph = field->cells[row][col] ? CELL_ALIVE : CELL_DEAD;
            mvaddch(row, col, glyph);
        }
    }
}

static void draw_status(int generation, int population, int delay) {
    mvprintw(ROWS, 0, "Gen:%-6d Pop:%-5d Delay:%-4dms  [A]faster  [Z]slower  [Space]quit ", generation,
             population, delay);
}

/* Return a new delay clamped to [DELAY_MIN, DELAY_MAX] for the pressed key. */
static int adjust_delay(int delay, int ch) {
    int result = delay;
    if (ch == KEY_FASTER_LOWER || ch == KEY_FASTER_UPPER) {
        result = delay - DELAY_STEP;
    }
    if (ch == KEY_SLOWER_LOWER || ch == KEY_SLOWER_UPPER) {
        result = delay + DELAY_STEP;
    }
    if (result < DELAY_MIN) {
        result = DELAY_MIN;
    }
    if (result > DELAY_MAX) {
        result = DELAY_MAX;
    }
    return result;
}

/* Bind ncurses to the controlling terminal so keyboard input still works
 * even when stdin has been redirected from a seed file. */
static SCREEN* init_screen(FILE* terminal) {
    SCREEN* screen = newterm(NULL, stdout, terminal);
    if (screen != NULL) {
        cbreak();
        noecho();
        curs_set(0);
        keypad(stdscr, TRUE);
    }
    return screen;
}

/* Main loop: render, wait for input up to "delay" ms, then evolve.
 * The two boards are swapped instead of copied, so each tick is O(rows*cols)
 * with no extra allocation. */
static void run_game(Field* current, Field* next) {
    int delay = DELAY_START;
    int generation = 0;
    int running = 1;
    while (running) {
        draw_field(current);
        draw_status(generation, count_population(current), delay);
        refresh();
        timeout(delay);
        int ch = getch();
        if (ch == KEY_QUIT) {
            running = 0;
        }
        delay = adjust_delay(delay, ch);
        if (running) {
            next_generation(current, next);
            Field* swap = current;
            current = next;
            next = swap;
            generation++;
        }
    }
}
