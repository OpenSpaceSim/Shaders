CC=clang++
CFLAGS+=-g
LDFLAGS+=-L. -lassimp -lGL -lGLU -lglut -lGLEW
OFILES=main.o shader.o util.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

openSpaceSim : $(OFILES)
	$(CC) -o openSpaceSim $(OFILES) $(LDFLAGS)
	
go:
	$(CC) -o openSpaceSim $(OFILES) $(LDFLAGS)
	./openSpaceSim
	
.PHONY: clean

clean:
	rm -f *.o *~ openSpaceSim
