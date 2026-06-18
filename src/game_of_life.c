/* The Game of Life -- Conway's cellular automaton (<stdio.h>-only version).
 *
 * This build depends on nothing but the C standard I/O library.
 *
 * The board is a self-contained (toroidal) grid of ROWS x COLS cells: the
 * neighbour of an edge cell is the cell on the opposite edge. A "filled" glyph
 * (O * 1 # ...) is a live cell; anything else (dots, spaces) is dead.
 *
 * Two ways to run it:
 *
 *   1. Interactive:   ./game_of_life patterns/glider.txt
 *      The seed is read from the file given on the command line, so stdin stays
 *      free for your controls. Type a key, then press Enter to apply it
 *      (standard line-buffered input, exactly like a turn-based terminal game).
 *
 *   2. Scriptable:    ./game_of_life < patterns/glider.txt
 *      With no file argument the seed is read from stdin; any control characters
 *      placed in the stream after the 25 seed lines drive the steps.
 *
 * Controls (read from stdin):
 *        A / a  -> faster (more generations advanced per step)
 *        Z / z  -> slower (fewer generations advanced per step)
 *        Space  -> end the game
 *        Enter (or any other key) -> advance one step at the current speed
 *
 * The code follows E. Dijkstra's structured programming principles: no goto,
 * no continue, no global variables, one entry and one exit per construct,
 * shallow nesting, and small single-purpose functions.
 */

#include <stdio.h>

#define ROWS 25
#define COLS 80

#define CELL_ALIVE 'O'
#define CELL_DEAD ' '

#define SPEED_START 1
#define SPEED_MIN 1
#define SPEED_MAX 20

#define KEY_FASTER_LOWER 'a'
#define KEY_FASTER_UPPER 'A'
#define KEY_SLOWER_LOWER 'z'
#define KEY_SLOWER_UPPER 'Z'
#define KEY_QUIT ' '

typedef struct {
    int cells[ROWS][COLS];
} Field;

int main(int argc, char** argv);
static int char_is_alive(int ch);
static void read_field(FILE* source, Field* field);
static int count_neighbours(const Field* field, int row, int col);
static void next_generation(const Field* current, Field* next);
static void step(Field** current, Field** next);
static int count_population(const Field* field);
static void clear_screen(void);
static void draw_field(const Field* field, int generation, int population, int speed);
static int adjust_speed(int speed, int ch);
static void run_game(Field* current, Field* next);

int main(int argc, char** argv) {
    Field current = {0};
    Field next = {0};
    FILE* seed = (argc > 1) ? fopen(argv[1], "r") : stdin;
    int status = 0;
    if (seed == NULL) {
        fprintf(stderr, "Error: cannot open seed file '%s'.\n", argv[1]);
        status = 1;
    } else {
        read_field(seed, &current);
        if (seed != stdin) {
            fclose(seed);
        }
        run_game(&current, &next);
    }
    return status;
}

/* Any cell marked with a "filled" glyph is treated as alive; everything
 * else (dots, zeros, spaces, ...) is treated as dead. */
static int char_is_alive(int ch) {
    return ch == '1' || ch == '*' || ch == 'O' || ch == 'o' || ch == '#' || ch == 'X' || ch == 'x' ||
           ch == '@';
}

/* Read the seed matrix from "source", one board row per text line. Reading
 * stops after ROWS lines so a redirected stream can still carry control input. */
static void read_field(FILE* source, Field* field) {
    int row = 0;
    int col = 0;
    int ch = 0;
    while (row < ROWS && (ch = fgetc(source)) != EOF) {
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

/* Advance one generation and swap the two boards, so each step is
 * O(rows*cols) with no copying and no dynamic allocation. */
static void step(Field** current, Field** next) {
    next_generation(*current, *next);
    Field* swap = *current;
    *current = *next;
    *next = swap;
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

/* Move the cursor home and clear the screen using ANSI escape codes
 * (plain characters written to stdout -- no system calls). */
static void clear_screen(void) { printf("\033[H\033[J"); }

static void draw_field(const Field* field, int generation, int population, int speed) {
    clear_screen();
    printf("The Game of Life | Gen: %-5d | Pop: %-4d | Speed: %d gen/step\n", generation, population, speed);
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            putchar(field->cells[row][col] ? CELL_ALIVE : CELL_DEAD);
        }
        putchar('\n');
    }
    printf("[A] faster   [Z] slower   [Space] quit\n");
}

/* Return a new speed clamped to [SPEED_MIN, SPEED_MAX] for the pressed key. */
static int adjust_speed(int speed, int ch) {
    int result = speed;
    if (ch == KEY_FASTER_LOWER || ch == KEY_FASTER_UPPER) {
        result = speed + 1;
    }
    if (ch == KEY_SLOWER_LOWER || ch == KEY_SLOWER_UPPER) {
        result = speed - 1;
    }
    if (result < SPEED_MIN) {
        result = SPEED_MIN;
    }
    if (result > SPEED_MAX) {
        result = SPEED_MAX;
    }
    return result;
}

/* Main loop: draw the colony, read one control character, then evolve by
 * "speed" generations. The game ends on Space or at end of input. */
static void run_game(Field* current, Field* next) {
    int speed = SPEED_START;
    int generation = 0;
    int running = 1;
    while (running) {
        draw_field(current, generation, count_population(current), speed);
        int ch = getchar();
        if (ch == EOF || ch == KEY_QUIT) {
            running = 0;
        } else if (ch == KEY_FASTER_LOWER || ch == KEY_FASTER_UPPER || ch == KEY_SLOWER_LOWER ||
                   ch == KEY_SLOWER_UPPER) {
            speed = adjust_speed(speed, ch);
        } else {
            for (int i = 0; i < speed; i++) {
                step(&current, &next);
            }
            generation += speed;
        }
    }
}
