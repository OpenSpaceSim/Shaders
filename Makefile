CC=clang++
CFLAGS+=-g
LDFLAGS+=-L. -lassimp -lGL -lGLU -lglut -lGLEW

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

openSpaceSim : main.o shader.o util.o
	$(CC) -o openSpaceSim main.o shader.o util.o $(LDFLAGS)
	
.PHONY: clean

clean:
	rm -f *.o *~ openSpaceSim
