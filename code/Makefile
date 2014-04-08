BINS=hw3b 

# There are much cleverer ways to deal with this in Makefiles.
maze.o : maze.c maze.h
hw3b.o : hw3b.c maze.h

show_maze2d : show_maze2d.o maze.o
hw3b : hw3b.o maze.o
	$(CC) -o $@ $(CFLAGS) $(CPPFLAGS) $^ $(LDFLAGS)

clean :
	rm -f *.o $(BINS)

