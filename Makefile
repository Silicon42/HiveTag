
CC := gcc
CFLAGS := -Wall -Wextra -g
SRC_DIR := src
OBJ_DIR := obj
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))
EXECUTABLES := gen_tags gen_tags_proto gen_chk_sym_shifter
COMMON_OBJS := obj/rs_gf16.o obj/gf16.o
COMMON_HEADERS := src/rs_gf16.h src/gf16.h
COMMON_DEPS := $(COMMON_OBJS) $(COMMON_HEADERS)

.PHONY: all
all: $(EXECUTABLES)

gen_tags: $(OBJ_DIR)/gen_tags.o $(COMMON_DEPS)
	$(CC) $(CFLAGS) -o $@ $^

gen_tags_proto: $(OBJ_DIR)/gen_tags_proto.o $(COMMON_DEPS)
	$(CC) $(CFLAGS) -o $@ $^

gen_chk_sym_shifter: $(OBJ_DIR)/gen_chk_sym_shifter.o $(COMMON_DEPS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f *.bin $(OBJ_DIR)/* *.exe