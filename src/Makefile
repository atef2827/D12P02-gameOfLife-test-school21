CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Werror
LDLIBS_NCURSES = -lncurses

SRC      = game_of_life.c
SRC_V2   = game_of_life_v2.c
TARGET   = game_of_life
TARGET_V2 = game_of_life_v2

all: $(TARGET) $(TARGET_V2)

# Dependency-free <stdio.h>-only build.
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# ncurses real-time build.
$(TARGET_V2): $(SRC_V2)
	$(CC) $(CFLAGS) $(SRC_V2) -o $(TARGET_V2) $(LDLIBS_NCURSES)

style:
	clang-format -n $(SRC) $(SRC_V2)

format:
	clang-format -i $(SRC) $(SRC_V2)

check:
	cppcheck --enable=all --suppress=missingIncludeSystem $(SRC) $(SRC_V2)

clean:
	rm -f $(TARGET) $(TARGET_V2)

.PHONY: all style format check clean
