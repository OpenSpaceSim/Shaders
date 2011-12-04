CC=clang++
CFLAGS+=-g
LDFLAGS+=-lGL -lGLU -lglut -lGLEW

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

openSpaceSim : main.o shader.o
	$(CC) -o openSpaceSim main.o shader.o $(LDFLAGS)
	
.PHONY: clean

clean:
	rm -f *.o *~ openSpaceSim
