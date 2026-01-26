NAME    := route_anim

CC      := gcc
CFLAGS  := -Wall -Wextra -Werror
LDFLAGS := -lm

SRCS    := main.c img.c
OBJS    := $(SRCS:.c=.o)

MAP_PNG := map.png
MAP_PPM := map.ppm

GIF_OUT := out.gif

W := 1386
H := 851

CONVERT := convert

.PHONY: all clean fclean re run gif ppm check

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

%.o: %.c img.h
	$(CC) $(CFLAGS) -c $< -o $@

ppm: $(MAP_PPM)

$(MAP_PPM): $(MAP_PNG)
	@echo "[PPM] generating $(MAP_PPM) from $(MAP_PNG) ..."
	@$(CONVERT) $(MAP_PNG) \
		-resize $(W)x$(H)\! \
		-depth 8 \
		-colorspace RGB \
		-define ppm:format=binary \
		$(MAP_PPM)
	@echo "[PPM] done: $(MAP_PPM)"

run: $(NAME) $(MAP_PPM)
	./$(NAME)

gif: run
	convert -delay 5 img*.ppm out.gif

check:
	@echo "---- files ----"
	@ls -l $(MAP_PNG) 2>/dev/null || true
	@ls -l $(MAP_PPM) 2>/dev/null || true
	@echo "---- ppm header (first 3 lines) ----"
	@head -n 3 $(MAP_PPM) 2>/dev/null || true

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)
	rm -f img*.ppm
	rm -f $(GIF_OUT)
	rm -f $(MAP_PPM)

re: fclean all
