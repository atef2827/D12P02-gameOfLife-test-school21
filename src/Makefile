CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Werror
LDLIBS  = -lncurses
TARGET  = game_of_life
SOURCE  = game_of_life.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE) -o $(TARGET) $(LDLIBS)

style:
	clang-format -n $(SOURCE)

format:
	clang-format -i $(SOURCE)

check:
	cppcheck --enable=all --suppress=missingIncludeSystem $(SOURCE)

clean:
	rm -f $(TARGET)

.PHONY: all style format check clean
