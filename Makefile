CC = gcc

OBJS = game.o
EXE  = game

CFLAGS = -Wall
LFLAGS = `pkg-config allegro-5 allegro_font-5 allegro_ttf-5 allegro_image-5 allegro_primitives-5 --libs --cflags`


all: $(EXE)

$(EXE): $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS) $(LFLAGS)

game.o: game.c
	$(CC) -c $< $(CFLAGS)

clean:
	@ rm -f $(EXE) $(OBJS)
